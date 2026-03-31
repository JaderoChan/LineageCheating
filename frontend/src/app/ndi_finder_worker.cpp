#include "ndi_finder_worker.h"

#include <Processing.NDI.Lib.h>

NdiFindWorker::NdiFindWorker(QObject* parent)
    : QObject(parent)
{}

void NdiFindWorker::doFind()
{
    QList<QPair<QString, QString>> results;

    NDIlib_find_instance_t ndiFinder = NDIlib_find_create_v2();
    if (!ndiFinder)
    {
        emit finished(results, false);
        return;
    }

    NDIlib_find_wait_for_sources(ndiFinder, 2000);

    uint32_t ndiSourcesNum = 0;
    const NDIlib_source_t* ndiSources = NDIlib_find_get_current_sources(ndiFinder, &ndiSourcesNum);

    for (uint32_t i = 0; i < ndiSourcesNum; ++i)
    {
        const NDIlib_source_t* source = ndiSources + i;
        results.append({QString(source->p_ndi_name), QString(source->p_url_address)});
    }

    NDIlib_find_destroy(ndiFinder);

    emit finished(results, true);
}
