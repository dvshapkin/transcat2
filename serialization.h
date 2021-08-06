#pragma once

#include <filesystem>
#include "map_renderer.h"

using namespace transcat;

void SerializeRenderSettings(pb3::TransportCatalogue &catalogue, const renderer::RenderSettings& settings);

renderer::RenderSettings DeserializeRenderSettings(pb3::TransportCatalogue &catalogue);
