#include <fstream>
#include <string_view>

#include "../transport_catalogue.h"
#include "../json_reader.h"

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

        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        db.Serialize(settings.file);
    }
    // Deserialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;
        RoutingSettings routing_settings;

        std::ifstream requests_in("process_requests_input1.json");

        json::Document doc = json::Load(requests_in);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        db.Deserialize(settings.file);

        query::JsonReader json_reader(db, renderer, routing_settings);
        auto stat_requests = json_reader.ParseStatRequests(doc);


        json_reader.WriteInfo(std::cout, stat_requests, routing_settings);

        //std::ifstream etalon_out("etalon_out1.json");
        //json::Document etalon = json::Load(etalon_out);
    }
}
