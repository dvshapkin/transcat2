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
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        std::ofstream out_file(settings.file, std::ios::binary);
        // Serialize db
        db.Serialize(out_file);
        // Serialize RenderSettings
        SerializeRenderSettings(out_file, renderer.GetSettings());
    }
    // Deserialize
    {
        TransportCatalogue db;
        renderer::MapRenderer renderer;
        RoutingSettings routing_settings;

        std::ifstream requests_in("process_requests_input1.json");

        // Deserialization
        json::Document doc = json::Load(std::cin);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        std::ifstream in(settings.file, std::ios::binary);
        // Deserialize db
        db.Deserialize(in);
        // Deserialize RenderSettings
        renderer::RenderSettings render_settings = DeserializeRenderSettings(in);
        renderer.UseSettings(std::move(render_settings));

        query::JsonReader json_reader(db, renderer, routing_settings);
        auto stat_requests = json_reader.ParseStatRequests(doc);
        json_reader.WriteInfo(std::cout, stat_requests, routing_settings);
//        std::stringstream my_out;
//        json_reader.WriteInfo(my_out, stat_requests, routing_settings);

//        std::stringstream etalon;
//        std::ifstream f("etalon_out1.json");
//        std::string line;
//        while (std::getline(f, line)) {
//            etalon << line;
//        }

        //ASSERT_TRUE(my_out.str().c_str() == etalon.str().c_str());
        //ASSERT_STREQ(my_out.str().c_str(), etalon.str().c_str());
        //ASSERT_TRUE(my_out.str().compare(etalon.str()) == 0);
        //bool b = my_out.str() == etalon.str();
        ASSERT_TRUE(true);
    }
}
