#include <iostream>
#include <fstream>
#include <string_view>

#include "transport_catalogue.h"
#include "json_reader.h"

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

        // Serialize
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        db.Serialize(settings.file);

    } else if (mode == "process_requests"sv) {

        // Deserialize
        json::Document doc = json::Load(std::cin);
        query::SerializationSettings settings = query::JsonReader::ParseSerializationSettings(doc);
        db.Deserialize(settings.file);

        query::JsonReader json_reader(db, renderer, routing_settings);
        auto stat_requests = json_reader.ParseStatRequests(doc);
        json_reader.WriteInfo(std::cout, stat_requests, routing_settings);

    } else {
        PrintUsage();
        return 1;
    }

    return 0;
}