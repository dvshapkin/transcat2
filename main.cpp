#include <iostream>
#include <fstream>
#include <string_view>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;
using namespace transcat;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    TransportCatalogue db;
    renderer::MapRenderer renderer;
    RoutingSettings routing_settings;

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        query::JsonReader json_reader(db, renderer, routing_settings);
        json::Document doc = json::Load(std::cin);
        json_reader.ReadData(doc);

        // Serialization
        pb3::TransportCatalogue catalogue;
        db.Serialize(catalogue);
        SerializeRenderSettings(catalogue, renderer.GetSettings());

        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        std::ofstream out_file(settings.file, std::ios::binary);
        catalogue.SerializeToOstream(&out_file);
        out_file.close();

    } else if (mode == "process_requests"sv) {

        // Deserialization
        json::Document doc = json::Load(std::cin);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        std::ifstream in_file(settings.file, std::ios::binary);

        pb3::TransportCatalogue catalogue;
        catalogue.ParseFromIstream(&in_file);
        db.Deserialize(catalogue);
        renderer::RenderSettings render_settings = DeserializeRenderSettings(catalogue);
        renderer.UseSettings(std::move(render_settings));

        query::JsonReader json_reader(db, renderer, routing_settings);
        auto stat_requests = json_reader.ParseStatRequests(doc);
        json_reader.WriteInfo(std::cout, stat_requests, routing_settings);

    } else {
        PrintUsage();
        return 1;
    }

    return 0;
}