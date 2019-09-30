#pragma once

#include <string>

/*
 * If Hodor needs to find it at run-time, it has to be extern "C".
 */
extern "C" {
int kv_init(void);
bool kv_get(const std::string &key, std::string &value);
void kv_put(const std::string &key, const std::string &value);
}
