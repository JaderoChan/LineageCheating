#include "tess_api.hpp"

TessAPI& TessAPI::getInstance()
{
    static TessAPI instance;
    return instance;
}

tesseract::TessBaseAPI& TessAPI::tessBaseAPI()
{
    return api_;
}

bool TessAPI::isInited() const
{
    return isInited_;
}

tesseract::TessBaseAPI& TessAPI::operator()()
{
    return api_;
}

TessAPI::TessAPI()
{
    // Disable `leptonica` debug output.
    setMsgSeverity(L_SEVERITY_NONE);

    // Disable `tesseract` debug output.
    api_.SetVariable("debug_file", "NUL");  // Redirect to `NUL` on Windows, And redirect to `NULL` on other platforms.
    api_.SetVariable("classify_debug_level", "0");
    api_.SetVariable("stopper_debug_level", "0");

    // Init for Chinese Traditional.
    if (api_.Init(nullptr, "chi_tra", tesseract::OEM_LSTM_ONLY))
    {
        isInited_ = true;
        api_.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
    }
}

TessAPI::~TessAPI()
{
    api_.End();
}
