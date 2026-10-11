#pragma once

#ifndef __vita__
#error Vita startup is only available on Vita targets
#endif

namespace corsixth_vita {

bool initialize();
bool log(const char* phase, const char* detail);
void log_errno(const char* phase, const char* operation, int error_number);
void shutdown();

}  // namespace corsixth_vita
