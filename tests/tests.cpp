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
        RoutingSettings routing_settings;

        std::ifstream base_in("make_base_input1.json");

        json::Document doc = json::Load(base_in);
        query::JsonReader json_reader(db, renderer, routing_settings);
        json_reader.ReadData(doc);

        // Serialization
        CatalogueSerializer serializer {db, renderer.GetSettings()};
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        serializer.SerializeTo(settings.file);
    }
    // Deserialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;
        RoutingSettings routing_settings;

        std::ifstream requests_in("process_requests_input1.json");

        // Deserialization
        json::Document doc = json::Load(requests_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        CatalogueDeserializer deserializer {db};
        deserializer.DeserializeFrom(settings.file);
        renderer.UseSettings(deserializer.GetRenderSettings());

        query::JsonReader json_reader(db, renderer, routing_settings);
        auto stat_requests = json_reader.ParseStatRequests(doc);
        //json_reader.WriteInfo(std::cout, stat_requests, routing_settings);
        std::stringstream my_out;
        json_reader.WriteInfo(my_out, stat_requests, routing_settings);
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
        RoutingSettings routing_settings;

        std::ifstream base_in("make_base_input2.json");

        json::Document doc = json::Load(base_in);
        query::JsonReader json_reader(db, renderer, routing_settings);
        json_reader.ReadData(doc);

        // Serialization
        CatalogueSerializer serializer {db, renderer.GetSettings()};
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        serializer.SerializeTo(settings.file);
    }
    // Deserialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;
        RoutingSettings routing_settings;

        std::ifstream requests_in("process_requests_input2.json");

        // Deserialization
        json::Document doc = json::Load(requests_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        CatalogueDeserializer deserializer {db};
        deserializer.DeserializeFrom(settings.file);
        renderer.UseSettings(deserializer.GetRenderSettings());

        query::JsonReader json_reader(db, renderer, routing_settings);
        auto stat_requests = json_reader.ParseStatRequests(doc);
        //json_reader.WriteInfo(std::cout, stat_requests, routing_settings);
        std::ofstream my_out("my_out2.json");
        json_reader.WriteInfo(my_out, stat_requests, routing_settings);


//        std::stringstream my_out;
//        json_reader.WriteInfo(my_out, stat_requests, routing_settings);
//        my_out << std::endl;
//
//        std::stringstream etalon;
//        std::ifstream f("etalon_out1.json");
//        std::string line;
//        while (std::getline(f, line)) {
//            etalon << line << std::endl;
//        }
//
//        ASSERT_EQ(my_out.str(), etalon.str());
    }
}

