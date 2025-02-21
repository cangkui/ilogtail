/*
 * Copyright 2022 iLogtail Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "LogFileReader.h"

namespace logtail {

class CommonRegLogFileReader : public LogFileReader {
public:
    CommonRegLogFileReader(const std::string& projectName,
                           const std::string& category,
                           const std::string& logPathDir,
                           const std::string& logPathFile,
                           int32_t tailLimit,
                           const std::string& timeFormat,
                           const std::string& topicFormat,
                           const std::string& groupTopic = "",
                           FileEncoding fileEncoding = ENCODING_UTF8,
                           bool discardUnmatch = true,
                           bool dockerFileFlag = false);

    void SetTimeKey(const std::string& timeKey);

    bool AddUserDefinedFormat(const std::string& regStr, const std::string& keys);

protected:
    bool ParseLogLine(StringView buffer,
                      sls_logs::LogGroup& logGroup,
                      ParseLogError& error,
                      LogtailTime& lastLogLineTime,
                      std::string& lastLogTimeStr,
                      uint32_t& logGroupSize);

    std::string mTimeKey;
    std::string mTimeFormat;
    std::vector<UserDefinedFormat> mUserDefinedFormat;
    std::vector<int32_t> mTimeIndex;

#ifdef APSARA_UNIT_TEST_MAIN
    friend class LogFileReaderUnittest;
    friend class CommonRegParseLogLineUnittest;
#endif
};

} // namespace logtail
