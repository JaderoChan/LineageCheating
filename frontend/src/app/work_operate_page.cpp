#include "work_operate_page.h"

WorkOperatePage::WorkOperatePage(const WorkConfig& config, QWidget* parent)
    : TrWidget(parent), config_(config)
{
    ui.setupUi(this);

    updateText();
}

WorkOperatePage::~WorkOperatePage()
{
    stop();
}

WorkConfig WorkOperatePage::getWorkConfig() const
{
    return config_;
}

void WorkOperatePage::setWorkConfig(const WorkConfig& config)
{
    config_ = config;
}

void WorkOperatePage::run()
{}

void WorkOperatePage::stop()
{}

void WorkOperatePage::isRunning() const
{}

void WorkOperatePage::configureMasterDevice()
{}

void WorkOperatePage::configureFootmanDevice()
{}

void WorkOperatePage::updateText()
{}
