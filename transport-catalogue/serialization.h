#pragma once

#include <transport_catalogue.pb.h>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"

#include <fstream>
#include <filesystem>
#include <iostream>


void MakeBase(std::istream& input_json);

std::optional<transport_base_processing::TransportCatalogue> DiserializeBase(const std::filesystem::path& path);

void ProcessRequest(std::istream& input_json);
