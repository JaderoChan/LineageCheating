#include "ocr.hpp"

class OcrLiteInstanceFactory
{
public:
    OcrLiteInstanceFactory()
    {
        bool success = instance_.initModels(
            "./models/ch_PP-OCRv3_det_infer.onnx",
            "./models/ch_ppocr_mobile_v2.0_cls_infer.onnx",
            "./models/ch_PP-OCRv3_rec_infer.onnx",
            "./models/ppocr_keys_v1.txt");
        if (!success)
            throw std::runtime_error("Failed to init OCR model.\n");
        instance_.initLogger(false, false, false);
        instance_.setNumThread(4);
        instance_.setGpuIndex(-1);
    }

    OcrLite& getInstance()
    {
        return instance_;
    }

private:
    OcrLiteInstanceFactory(OcrLiteInstanceFactory&) = delete;
    OcrLiteInstanceFactory& operator=(const OcrLiteInstanceFactory&) = delete;

    OcrLite instance_;
};

OcrLite& getOcrLiteInstance()
{
    static OcrLiteInstanceFactory instance;
    return instance.getInstance();
}

std::string getImageText(const cv::Mat& img)
{
    try
    {
        auto& ocrLite = getOcrLiteInstance();
        auto result = ocrLite.detect(img, 50, 1024, 0.35, 0.3, 1.6, false, false);
        return result.strRes;
    }
    catch (const std::exception& e)
    {
        printf("Failed to detect text: %s", e.what());
        return std::string();
    }
}
