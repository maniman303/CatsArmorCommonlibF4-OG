#include "PCH.h"

void REX::ERROR(std::string message)
{
    logger::error("{}", message);
}

void REX::WARN(std::string message)
{
    logger::warn("{}", message);
}

void REX::INFO(std::string message)
{
    logger::info("{}", message);
}