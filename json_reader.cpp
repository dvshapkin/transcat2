#include <unordered_set>
#include <sstream>
#include <fstream>

#include "domain.h"
#include "json_reader.h"
#include "request_handler.h"
#include "json_builder.h"

#include "graph.h"
#include "router.h"
#include "profile.h"

namespace transcat::query {

    using namespace std::string_literals;

    StatRequestType StatRequestTypeFromString(const std::string &type_name) {
        if (type_name == "Bus"s) {
            return StatRequestType::Bus;
        } else if (type_name == "Stop"s) {
            return StatRequestType::Stop;
        } else if (type_name == "Map"s) {
            return StatRequestType::Map;
        } else { //if (type_name == "Route"s) {
            return StatRequestType::Route;
        }
    }

    ///////////////////////// JsonReader /////////////////////////////////

    JsonReader::JsonReader(TransportCatalogue &db, renderer::MapRenderer &renderer, RoutingSettings &routing_settings)
        : db_(db), renderer_(renderer), routing_settings_(routing_settings) {
    }

    void JsonReader::ReadData(const json::Document &document) {
        ParseBaseRequests(document);
        ParseRenderSettings(document);
        ParseRoutingSettings(document);
    }

    void JsonReader::WriteInfo(std::ostream &out, const std::vector<query::StatRequest> &requests,
                               RoutingSettings settings) const {
        RequestHandler handler{db_, renderer_, settings, db_.EvaluateVertexCount()};
        graph::Router<double> router(handler.GetRouteGraph());
        json::Array responses;
        for (const auto &request: requests) {
            switch (request.type) {
                case StatRequestType::Bus:
                    WriteBusInfo(handler, responses, request);
                    break;
                case StatRequestType::Stop:
                    WriteStopInfo(handler, responses, request);
                    break;
                case StatRequestType::Map:
                    WriteMapInfo(handler, responses, request);
                    break;
                case StatRequestType::Route:
                    WriteRouteInfo(handler, router, responses, request);
                    break;
            }
        }
        json::Print(json::Document(std::move(responses)), out);
    }

    void JsonReader::ParseBaseRequests(const json::Document &document) const {
        json::Dict data = document.GetRoot().AsDict();
        if (data.count("base_requests"s)) {
            json::Array requests = data.at("base_requests"s).AsArray();

            // для временного хранения запросов по маршрутам
            std::vector<json::Dict> bus_queries;

            // для временного хранения части запроса с информацией о дистанциях
            std::unordered_map<StopPtr, json::Dict> distances_for_stop;

            for (const auto &node: requests) {
                const json::Dict &request = node.AsDict();
                const std::string &request_type = request.at("type"s).AsString();

                if (request_type == "Stop"s) {
                    Stop stop{
                            request.at("name"s).AsString(),
                            request.at("latitude"s).AsDouble(),
                            request.at("longitude"s).AsDouble()
                    };
                    db_.AddStop(stop);
                    distances_for_stop.insert(
                            {db_.GetStop(stop.name), request.at("road_distances"s).AsDict()}
                    );
                }

                if (request_type == "Bus"s) {
                    // сохраним для дальнейшей обработки
                    bus_queries.push_back(request);
                }
            }

            // отложенная обработка дистанций м/у остановками
            UpdateDistances(distances_for_stop);

            // отложенная обработка маршрутов
            UpdateRoutes(bus_queries);
        }
    }

    std::vector<StatRequest> JsonReader::ParseStatRequests(const json::Document &document) const {
        json::Dict data = document.GetRoot().AsDict();
        std::vector<StatRequest> result;

        if (data.count("stat_requests"s)) {
            json::Array requests = data.at("stat_requests"s).AsArray();
            result.reserve(requests.size());

            for (const auto &node: requests) {
                const json::Dict &request = node.AsDict();
                const auto request_type = StatRequestTypeFromString(request.at("type"s).AsString());
                result.push_back({
                                         request.at("id"s).AsInt(),
                                         request_type,
                                         std::nullopt
                                 });
                switch (request_type) {
                    case StatRequestType::Bus:
                    case StatRequestType::Stop:
                        result.back().data = request.at("name"s).AsString();
                        break;
                    case StatRequestType::Route:
                        result.back().data = StopPair{
                                db_.GetStop(request.at("from"s).AsString()),
                                db_.GetStop(request.at("to"s).AsString())
                        };
                        break;
                    case StatRequestType::Map:
                        break;
                }
            }
        }

        return result;
    }

    void JsonReader::ParseRenderSettings(const json::Document &document) const {
        json::Dict data = document.GetRoot().AsDict();

        if (data.count("render_settings"s)) {
            json::Dict render_settings = data.at("render_settings"s).AsDict();

            auto bus_label_offset_node = render_settings.at("bus_label_offset"s).AsArray();
            auto stop_label_offset_node = render_settings.at("stop_label_offset"s).AsArray();
            auto color_palette_node = render_settings.at("color_palette"s).AsArray();

            std::vector<svg::Color> color_palette;
            for (const auto &node: color_palette_node) {
                color_palette.push_back(ParseColor(node));
            }

            renderer::RenderSettings settings{
                    render_settings.at("width"s).AsDouble(),
                    render_settings.at("height"s).AsDouble(),
                    render_settings.at("padding"s).AsDouble(),
                    render_settings.at("line_width"s).AsDouble(),
                    render_settings.at("stop_radius"s).AsDouble(),
                    render_settings.at("bus_label_font_size"s).AsInt(),
                    {
                            bus_label_offset_node[0].AsDouble(),
                            bus_label_offset_node[1].AsDouble()
                    },
                    render_settings.at("stop_label_font_size"s).AsInt(),
                    {
                            stop_label_offset_node[0].AsDouble(),
                            stop_label_offset_node[1].AsDouble()
                    },
                    ParseColor(render_settings.at("underlayer_color"s)),
                    render_settings.at("underlayer_width"s).AsDouble(),
                    std::move(color_palette)
            };

            renderer_.UseSettings(std::move(settings));
        }
    }

    void JsonReader::ParseRoutingSettings(const json::Document &document) const {
        json::Dict data = document.GetRoot().AsDict();
        if (data.count("routing_settings"s)) {
            json::Dict routing_settings = data.at("routing_settings"s).AsDict();
//            return {
//                    routing_settings.at("bus_wait_time"s).AsInt(),
//                    routing_settings.at("bus_velocity"s).AsDouble()
//            };
            routing_settings_ = {
                    routing_settings.at("bus_wait_time"s).AsInt(),
                    routing_settings.at("bus_velocity"s).AsDouble()
            };
        }
        //return {};
    }

    SerializationSettings JsonReader::ParseSerializationSettings(const json::Document &document) {
        json::Dict data = document.GetRoot().AsDict();
        if (data.count("serialization_settings"s)) {
            json::Dict serialization_settings = data.at("serialization_settings"s).AsDict();
            return {serialization_settings.at("file"s).AsString()};
        }
        return {};
    }

    void JsonReader::UpdateDistances(const std::unordered_map<StopPtr, json::Dict> &distances) const {
        for (const auto&[from, distances_to]: distances) {
            for (const auto&[stop_name, distance]: distances_to) {
                db_.SetDistance({from, db_.GetStop(stop_name)}, distance.AsInt());
            }
        }
    }

    void JsonReader::UpdateRoutes(const std::vector<json::Dict> &bus_queries) const {
        for (const auto &request: bus_queries) {
            bool is_roundtrip = request.at("is_roundtrip"s).AsBool();

            Route route;
            std::deque<StopPtr> temp; // временное хранение остановок для некольцевого маршрута
            std::unordered_set<StopPtr> unique_stops;

            json::Array stops = request.at("stops"s).AsArray();
            Stop *start_stop = nullptr;
            Stop *end_stop = nullptr;
            if (!stops.empty()) {
                start_stop = const_cast<Stop *>(db_.GetStop(stops.front().AsString()));
                end_stop = const_cast<Stop *>(db_.GetStop(stops.back().AsString()));
                for (const auto &node: stops) {
                    StopPtr p_stop = db_.GetStop(node.AsString());
                    if (p_stop) {
                        route.push_back(p_stop);
                        unique_stops.emplace(p_stop);

                        // если маршрут не кольцевой - продублируем маршрут в обратном порядке
                        if (!is_roundtrip) {
                            temp.push_front(p_stop);
                        }
                    }
                }
            }

            // дополним не кольцевой маршрут
            if (temp.size() != 0) {
                for (auto it = temp.cbegin() + 1; it != temp.cend(); ++it) {
                    route.push_back(*it);
                }
            }

            Bus bus{
                    request.at("name"s).AsString(),
                    route,
                    unique_stops.size(),
                    is_roundtrip,
                    start_stop,
                    end_stop
            };
            db_.AddBus(bus);
        }
    }

    svg::Color JsonReader::ParseColor(const json::Node &color) const {
        if (color.IsString()) {
            return color.AsString();
        } else {
            auto arr = color.AsArray();
            if (arr.size() == 3) {
                return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
            }
            if (arr.size() == 4) {
                return svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
            }
            throw std::logic_error("Not a color"s);
        }
    }

    void
    JsonReader::WriteBusInfo(const RequestHandler &handler, json::Array &responses, const StatRequest &request) const {
        auto responce = json::Builder();
        responce.StartDict()
                .Key("request_id"s).Value(request.id);
        auto stat = handler.GetBusStat(std::get<std::string>(request.data));
        if (stat) {
            responce.Key("curvature"s).Value(stat->curvature);
            responce.Key("route_length"s).Value(static_cast<double>(stat->route_length));
            responce.Key("stop_count"s).Value(stat->stop_count);
            responce.Key("unique_stop_count"s).Value(stat->unique_stop_count);
        } else {
            responce.Key("error_message"s).Value("not found"s);
        }
        responce.EndDict();
        responses.push_back(responce.Build());
    }

    void
    JsonReader::WriteStopInfo(const RequestHandler &handler, json::Array &responses, const StatRequest &request) const {
        auto responce = json::Builder();
        responce.StartDict()
                .Key("request_id"s).Value(request.id);

        auto name = std::get<std::string>(request.data);
        if (handler.IsStopExists(name)) {
            json::Array buses;
            if (handler.IsStopInRoutes(name)) {
                auto buses_by_stop = handler.GetBusesByStop(name);
                if (!buses_by_stop->empty()) {
                    buses.reserve(buses_by_stop->size());
                    for (const auto p_bus: *buses_by_stop) {
                        buses.push_back(p_bus->name);
                    }
                }
            }
            responce.Key("buses"s).Value(buses);
        } else {
            responce.Key("error_message"s).Value("not found"s);
        }
        responce.EndDict();
        responses.push_back(responce.Build());
    }

    void
    JsonReader::WriteMapInfo(const RequestHandler &handler, json::Array &responses, const StatRequest &request) const {
        svg::Document doc = handler.RenderMap();
        std::stringstream map;
        doc.Render(map);

//        //------------------------------------------------
//        std::ofstream f("map.svg", std::ios::out | std::ios::trunc);
//        if (f.is_open()) {
//            f << map.str() << std::endl;
//            f.close();
//        }
//        //------------------------------------------------

        auto responce = json::Builder()
                .StartDict()
                .Key("request_id"s).Value(request.id)
                .Key("map"s).Value(map.str())
                .EndDict()
                .Build();
        responses.push_back(std::move(responce));
    }

    void JsonReader::WriteRouteInfo(const RequestHandler &handler, const graph::Router<double> &router,
                                    json::Array &responses, const StatRequest &request) const {
        StopPair from_to = std::get<StopPair>(request.data);
        auto opt_route_info = router.BuildRoute(handler.GetVertexForStop(from_to.from),
                                                handler.GetVertexForStop(from_to.to));

        json::Dict resp;
        resp["request_id"] = request.id;
        if (opt_route_info) {
            json::Array items;
            if (opt_route_info && !opt_route_info->edges.empty()) {
                MakeRouteItems(handler, *opt_route_info, items);
            }
            resp["total_time"] = opt_route_info->weight;
            resp["items"] = items;
        } else {
            resp["error_message"] = "not found"s;
        }

        responses.push_back(json::Builder().Value(resp).Build());
    }

    void JsonReader::MakeRouteItems(const RequestHandler &handler,
                                      const graph::Router<double>::RouteInfo &route_info, json::Array &items) const {

        auto settings = handler.GetRoutingSettings();

        for (graph::EdgeId edge_id: route_info.edges) {
            auto edge = handler.GetRouteGraph().GetEdge(edge_id);
            const Bus *p_bus = static_cast<const Bus *>(edge.p_bus);

            json::Dict item_wait = MakeWaitItem(handler.GetStopForVertex(edge.from)->name, settings.bus_wait_time);
            items.push_back(std::move(item_wait));

            json::Dict item_bus = MakeBusItem(p_bus->name, edge.span_count, edge.weight - settings.bus_wait_time);
            items.push_back(std::move(item_bus));
        }
    }

    json::Dict JsonReader::MakeWaitItem(const std::string &stop_name, int wait_time) const {
        json::Dict item_wait;
        item_wait["type"] = "Wait"s;
        item_wait["stop_name"] = stop_name;
        item_wait["time"] = wait_time;
        return item_wait;
    }

    json::Dict JsonReader::MakeBusItem(const std::string &bus_name, int span_count, double time) const {
        json::Dict item_bus;
        item_bus["type"] = "Bus"s;
        item_bus["bus"] = bus_name;
        item_bus["span_count"] = span_count;
        item_bus["time"] = time;
        return item_bus;
    }

} // namespace transcat::query


