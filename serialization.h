#pragma once

#include "map_renderer.h"

using namespace transcat;

void SerializeRenderSettings(std::ofstream &out, const renderer::RenderSettings& settings);

renderer::RenderSettings DeserializeRenderSettings(std::ifstream &in);
