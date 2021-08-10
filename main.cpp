#include <iostream>
#include <fstream>
#include <string_view>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;
using namespace transcat;

void PrintUsage(std::ostream &stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    json::Document doc = json::Load(std::cin);
    query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);

    TransportCatalogue db;
    renderer::MapRenderer renderer;

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        // Заполним БД
        query::JsonReader json_reader(db, renderer);
        json_reader.ReadData(doc);

        // Построим граф маршрутов
        RequestHandler handler{db, renderer, json_reader.GetRoutingSettings(), db.EvaluateVertexCount()};
        graph::Router<double> router(handler.GetRouteGraph());

        // Сереализуем
        CatalogueSerializer serializer{db, renderer.GetSettings(), json_reader.GetRoutingSettings(),
                                       handler.GetRouteGraph(), router.GetRoutesInternalData()};
        serializer.SerializeTo(settings.file);

    } else if (mode == "process_requests"sv) {

        // Десериализуем
        CatalogueDeserializer deserializer{db};
        deserializer.DeserializeFrom(settings.file);
        renderer.UseSettings(deserializer.GetRenderSettings());

        // Обработка запросов
        query::JsonReader json_reader(db, renderer);
        json_reader.SetRoutingSettings(deserializer.GetRoutingSettings());
        auto stat_requests = json_reader.ParseStatRequests(doc);
        json_reader.WriteInfo(std::cout, stat_requests, deserializer.GetRouteGraph(),
                              deserializer.GetRoutesInternalData());

    } else {
        PrintUsage();
        return 1;
    }

    return 0;
}
