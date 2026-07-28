#include "PCH.h"

void REX::ERROR(const std::string_view message)
{
    logger::error("{}", message);
}

void REX::WARN(const std::string_view message)
{
    logger::warn("{}", message);
}

void REX::INFO(const std::string_view message)
{
    logger::info("{}", message);
}