#include "experimentwizard.h"

#include "wizardstartpage.h"
#include "wizardchirpconfigpage.h"
#include "wizardftmwconfigpage.h"
#include "wizardsummarypage.h"
#include "wizardpulseconfigpage.h"
#include "wizardlifconfigpage.h"
#include "batchsingle.h"

ExperimentWizard::ExperimentWizard(QWidget *parent) :
    QWizard(parent)
{
    setWindowTitle(QString("Experiment Setup"));

    p_startPage = new WizardStartPage(this);
    p_chirpConfigPage = new WizardChirpConfigPage(this);
    p_ftmwConfigPage = new WizardFtmwConfigPage(this);
    p_pulseConfigPage = new WizardPulseConfigPage(this);
    p_summaryPage = new WizardSummaryPage(this);
    p_lifConfigPage = new WizardLifConfigPage(this);
    connect(this,&ExperimentWizard::newTrace,p_lifConfigPage,&WizardLifConfigPage::newTrace);
    connect(this,&ExperimentWizard::scopeConfigChanged,p_lifConfigPage,&WizardLifConfigPage::scopeConfigChanged);
    connect(p_lifConfigPage,&WizardLifConfigPage::updateScope,this,&ExperimentWizard::updateScope);


    setPage(StartPage,p_startPage);
    setPage(ChirpConfigPage,p_chirpConfigPage);
    setPage(FtmwConfigPage,p_ftmwConfigPage);
    setPage(PulseConfigPage,p_pulseConfigPage);
    setPage(LifConfigPage,p_lifConfigPage);
    setPage(SummaryPage,p_summaryPage);
}

ExperimentWizard::~ExperimentWizard()
{ 
}

void ExperimentWizard::setPulseConfig(const PulseGenConfig c)
{
    p_pulseConfigPage->setConfig(c);
}

void ExperimentWizard::setFlowConfig(const FlowConfig c)
{
    d_flowConfig = c;
}

Experiment ExperimentWizard::getExperiment() const
{
    Experiment exp;

    FtmwConfig ftc = p_ftmwConfigPage->getFtmwConfig();
    if(p_startPage->ftmwEnabled())
    {
        ftc.setEnabled();

        ChirpConfig cc = p_chirpConfigPage->getChirpConfig();
        ftc.setChirpConfig(cc);
    }

    exp.setFtmwConfig(ftc);
    exp.setPulseGenConfig(p_pulseConfigPage->getConfig());
    exp.setFlowConfig(d_flowConfig);
    exp.setTimeDataInterval(p_startPage->auxDataInterval());

    return exp;
}

BatchManager *ExperimentWizard::getBatchManager() const
{
    return new BatchSingle(getExperiment());
}

void ExperimentWizard::saveToSettings()
{
    if(p_startPage->ftmwEnabled())
    {
        p_chirpConfigPage->saveToSettings();
        p_ftmwConfigPage->saveToSettings();
    }

    if(p_startPage->lifEnabled())
        p_lifConfigPage->saveToSettings();
}

