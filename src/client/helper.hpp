#pragma once

#include <string>
#include <stdexcept>
#include <iostream>

// Function to get the current date and time in the format "DD/MM/YYYY - HH:MMAM/PM"
const std::string date_time();

// Function to print an error message and throw a runtime error
[[noreturn]] void throwError(const std::string& errorMessage);
