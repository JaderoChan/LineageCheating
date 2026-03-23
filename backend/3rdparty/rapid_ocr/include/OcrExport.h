#ifndef __OCR_EXPORT_H__
#define __OCR_EXPORT_H__

#ifdef WIN32
    #ifdef __CLIB__
        #define _QM_OCR_API __declspec(dllexport)
    #else
        #define _QM_OCR_API __declspec(dllimport)
    #endif
#else
    #define _QM_OCR_API
#endif

#endif // !__OCR_EXPORT_H__
