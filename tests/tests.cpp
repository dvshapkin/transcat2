#include <fstream>
#include <sstream>
#include <string_view>

#include "../transport_catalogue.h"
#include "../json_reader.h"
#include "../serialization.h"

#include "gtest/gtest.h"

using namespace std::literals;
using namespace transcat;

TEST(SERIALIZE_SUITE, Test_01) {
    // Serialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;

        std::ifstream base_in("make_base_input1.json");

        json::Document doc = json::Load(base_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);

        // Заполним БД
        query::JsonReader json_reader(db, renderer);
        json_reader.ReadData(doc);

        // Построим граф маршрутов
        RequestHandler handler{db, renderer, json_reader.GetRoutingSettings(), db.EvaluateVertexCount()};
        graph::Router<double> router(handler.GetRouteGraph());

        // Serialization
        CatalogueSerializer serializer{db, renderer.GetSettings(), json_reader.GetRoutingSettings(),
                                       handler.GetRouteGraph(), router.GetRoutesInternalData()};
        serializer.SerializeTo(settings.file);
    }
    // Deserialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;

        std::ifstream requests_in("process_requests_input1.json");

        json::Document doc = json::Load(requests_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);

        // Десериализуем
        CatalogueDeserializer deserializer{db};
        deserializer.DeserializeFrom(settings.file);
        renderer.UseSettings(deserializer.GetRenderSettings());

        query::JsonReader json_reader(db, renderer);
        json_reader.SetRoutingSettings(deserializer.GetRoutingSettings());
        auto stat_requests = json_reader.ParseStatRequests(doc);
        //json_reader.WriteInfo(std::cout, stat_requests, deserializer.GetRouteGraph());
        std::stringstream my_out;
        json_reader.WriteInfo(my_out, stat_requests, deserializer.GetRouteGraph(),
                              deserializer.GetRoutesInternalData());
        my_out << std::endl;

        std::stringstream etalon;
        std::ifstream f("etalon_out1.json");
        std::string line;
        while (std::getline(f, line)) {
            etalon << line << std::endl;
        }

        ASSERT_EQ(my_out.str(), etalon.str());
    }
}

TEST(SERIALIZE_SUITE, Test_02) {
    // Serialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;

        std::ifstream base_in("make_base_input2.json");

        json::Document doc = json::Load(base_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);

        // Заполним БД
        query::JsonReader json_reader(db, renderer);
        json_reader.ReadData(doc);

        // Построим граф маршрутов
        RequestHandler handler{db, renderer, json_reader.GetRoutingSettings(), db.EvaluateVertexCount()};
        graph::Router<double> router(handler.GetRouteGraph());

        // Serialization
        CatalogueSerializer serializer{db, renderer.GetSettings(), json_reader.GetRoutingSettings(),
                                       handler.GetRouteGraph(), router.GetRoutesInternalData()};
        serializer.SerializeTo(settings.file);
    }
    // Deserialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;

        std::ifstream requests_in("process_requests_input2.json");

        // Deserialization
        json::Document doc = json::Load(requests_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        CatalogueDeserializer deserializer{db};
        deserializer.DeserializeFrom(settings.file);
        renderer.UseSettings(deserializer.GetRenderSettings());

        query::JsonReader json_reader(db, renderer);
        json_reader.SetRoutingSettings(deserializer.GetRoutingSettings());
        auto stat_requests = json_reader.ParseStatRequests(doc);

        //json_reader.WriteInfo(std::cout, stat_requests, deserializer.GetRouteGraph());
        std::ofstream my_out("my_out2.json");
        json_reader.WriteInfo(my_out, stat_requests, deserializer.GetRouteGraph(),
                              deserializer.GetRoutesInternalData());
    }
}

TEST(SERIALIZE_SUITE, Test_03) {
    // Serialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;

        std::ifstream base_in("make_base_input3.json");

        json::Document doc = json::Load(base_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);

        // Заполним БД
        query::JsonReader json_reader(db, renderer);
        json_reader.ReadData(doc);

        // Построим граф маршрутов
        RequestHandler handler{db, renderer, json_reader.GetRoutingSettings(), db.EvaluateVertexCount()};
        graph::Router<double> router(handler.GetRouteGraph());

        // Serialization
        //auto rid = router.GetRoutesInternalData();
        CatalogueSerializer serializer{db, renderer.GetSettings(), json_reader.GetRoutingSettings(),
                                       handler.GetRouteGraph(), router.GetRoutesInternalData()};
        serializer.SerializeTo(settings.file);
    }
    // Deserialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;

        std::ifstream requests_in("process_requests_input3.json");

        // Deserialization
        json::Document doc = json::Load(requests_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        CatalogueDeserializer deserializer{db};
        deserializer.DeserializeFrom(settings.file);
        renderer.UseSettings(deserializer.GetRenderSettings());

        query::JsonReader json_reader(db, renderer);
        json_reader.SetRoutingSettings(deserializer.GetRoutingSettings());
        auto stat_requests = json_reader.ParseStatRequests(doc);

        //json_reader.WriteInfo(std::cout, stat_requests, deserializer.GetRouteGraph());
        std::ofstream my_out("my_out3.json");
        json_reader.WriteInfo(my_out, stat_requests, deserializer.GetRouteGraph(),
                              deserializer.GetRoutesInternalData());
    }
}

TEST(SERIALIZE_SUITE, Test_10) {
    // Serialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;

        std::ifstream base_in("make_base_input10.json");

        json::Document doc = json::Load(base_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);

        // Заполним БД
        query::JsonReader json_reader(db, renderer);
        json_reader.ReadData(doc);

        // Построим граф маршрутов
        RequestHandler handler{db, renderer, json_reader.GetRoutingSettings(), db.EvaluateVertexCount()};
        graph::Router<double> router(handler.GetRouteGraph());

        // Serialization
        //auto rid = router.GetRoutesInternalData();
        CatalogueSerializer serializer{db, renderer.GetSettings(), json_reader.GetRoutingSettings(),
                                       handler.GetRouteGraph(), router.GetRoutesInternalData()};
        serializer.SerializeTo(settings.file);
    }
    // Deserialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;

        std::ifstream requests_in("process_requests_input10.json");

        // Deserialization
        json::Document doc = json::Load(requests_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        CatalogueDeserializer deserializer{db};
        deserializer.DeserializeFrom(settings.file);
        renderer.UseSettings(deserializer.GetRenderSettings());

        query::JsonReader json_reader(db, renderer);
        json_reader.SetRoutingSettings(deserializer.GetRoutingSettings());
        auto stat_requests = json_reader.ParseStatRequests(doc);

        //json_reader.WriteInfo(std::cout, stat_requests, deserializer.GetRouteGraph());
        std::ofstream my_out("my_out10.json");
        json_reader.WriteInfo(my_out, stat_requests, deserializer.GetRouteGraph(),
                              deserializer.GetRoutesInternalData());
    }
}