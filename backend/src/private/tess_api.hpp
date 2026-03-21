#pragma once

#include <string>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

class TessAPI
{
public:
    static TessAPI& getInstance();

    bool isInited() const;

    tesseract::TessBaseAPI& tessBaseAPI();
    tesseract::TessBaseAPI& operator()();

private:
    TessAPI();
    ~TessAPI();

    TessAPI(const TessAPI&) = delete;
    TessAPI& operator=(const TessAPI&) = delete;

    bool isInited_ = false;
    tesseract::TessBaseAPI api_;
};
