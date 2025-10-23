#include "deviceargsetform.h"
#include "ui_deviceargsetform.h"
#include "settings.h"
#include <QKeyEvent>
#include <QToolTip>


DeviceArgSetForm::DeviceArgSetForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeviceArgSetForm)
{
    ui->setupUi(this);
    ui->setWidget->hide();

    ui->exposureTime->installEventFilter(this);
    ui->frameRate->installEventFilter(this);
    ui->timeFilterV->installEventFilter(this);
    ui->flyFilterV->installEventFilter(this);
    ui->confFilterV->installEventFilter(this);

    connect(ui->OK, &QPushButton::clicked, this, [this]{
        if(ui->password->text() == "admin" || ui->password->text() == "lw-admin")
        {
            ui->passwordWidget->hide();
            ui->setWidget->show();
        }
        else
        {
            ui->info->setText(tr("密码错误！！！"));
        }
    });

    connect(ui->password, &QLineEdit::returnPressed, this, [this]{
        if(ui->password->text() == "admin" || ui->password->text() == "lw-admin")
        {
            ui->passwordWidget->hide();
            ui->setWidget->show();
        }
        else
        {
            ui->info->setText(tr("密码错误！！！"));
        }
    });

    connect(ui->quit, &QPushButton::clicked, this, [this]{
        this->hide();
        ui->setWidget->hide();
        ui->password->clear();
        ui->passwordWidget->show();
    });

    connect(ui->saveConfig, &QPushButton::clicked, this, [this]{ emit saveDeviceConfig(); });

    connect(ui->NO, &QPushButton::clicked, this, [this]{
        this->hide();
        ui->password->clear();
    });

    connect(ui->password, &QLineEdit::textChanged, this, [this](QString){
        if(!ui->info->text().isEmpty()) ui->info->setText("");
    });

    connect(ui->flyFilterB, &QCheckBox::checkStateChanged, this, [this](const Qt::CheckState &arg1){
        emit setFlyPixeFilter(arg1 == Qt::CheckState::Checked, ui->flyFilterV->value());
    });

    connect(ui->confFilterB, &QCheckBox::checkStateChanged, this, [this](const Qt::CheckState &arg1){
        emit setConfidenceFilter(arg1 == Qt::CheckState::Checked, ui->confFilterV->value());
    });

    connect(ui->timeFilterB, &QCheckBox::checkStateChanged, this, [this](const Qt::CheckState &arg1){
        emit setTimeFilter(arg1 == Qt::CheckState::Checked, ui->timeFilterV->value());
    });

    connect(ui->timeFilterV_, &QComboBox::currentIndexChanged, this, [this](int arg){
        emit setTimeFilter_(ui->timeFilterB_->isChecked(), arg * 2 + 3);
    });
    connect(ui->timeFilterB_, &QCheckBox::checkStateChanged, this, [this](const Qt::CheckState &arg1){
        emit setTimeFilter_(arg1 == Qt::CheckState::Checked, ui->timeFilterV_->currentIndex() * 2 + 3);
    });

    connect(ui->spatialFilterV, &QComboBox::currentIndexChanged, this, [this](int arg){
        emit setSpatialFilter(ui->spatialFilterB->isChecked(), arg * 2 + 3);
    });
    connect(ui->spatialFilterB, &QCheckBox::checkStateChanged, this, [this](const Qt::CheckState &arg1){
        emit setSpatialFilter(arg1 == Qt::CheckState::Checked, ui->spatialFilterV->currentIndex() * 2 + 3);
    });

    connect(ui->isAutoReboot, &QCheckBox::checkStateChanged, this, [this](const Qt::CheckState &arg1){
        config.isAutoReboot = (arg1 == Qt::CheckState::Checked);
    });
}

DeviceArgSetForm::~DeviceArgSetForm()
{
    delete ui;
}

void DeviceArgSetForm::setArgUI()
{
    this->blockSignals(true);

    ui->exposureTime->setValue(config.exposure[0]);
    ui->frameRate->setValue(config.frameRate);
    ui->timeFilterB->setChecked(config.isTimeFilter);
    ui->timeFilterV->setValue(config.timeFilterValue);
    ui->timeFilterB_->setChecked(config.isTimeFilter_);
    ui->timeFilterV_->setCurrentIndex((config.timeFilterValue_ - 3) / 2);
    ui->spatialFilterB->setChecked(config.isSpatialFilter);
    ui->spatialFilterV->setCurrentIndex((config.spatialFilterValue - 3) / 2);
    ui->flyFilterB->setChecked(config.isFlyPixeFilter);
    ui->flyFilterV->setValue(config.flyPixeFilterValue);
    ui->confFilterB->setChecked(config.isConfFilter);
    ui->confFilterV->setValue(config.confFilterValue);

    ui->isAutoReboot->setChecked(config.isAutoReboot);

    this->blockSignals(false);
}

void DeviceArgSetForm::setExposureLevel(int val)
{
    val *= 40;
    ui->exposureTime->setValue(val);
    emit setExposure(val);
}

bool DeviceArgSetForm::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {

            if(obj == ui->exposureTime)
            {
                ui->exposureTime->setValue(ui->exposureTime->value());
                emit setExposure(ui->exposureTime->value());
            }

            if(obj == ui->frameRate)
            {
                ui->frameRate->setValue(ui->frameRate->value());
                emit setFrameRate(ui->frameRate->value());
            }

            if(obj == ui->flyFilterV)
            {
                ui->flyFilterV->setValue(ui->flyFilterV->value());
                emit setFlyPixeFilter(ui->flyFilterB->isChecked(), ui->flyFilterV->value());
            }

            if(obj == ui->confFilterV)
            {
                ui->confFilterV->setValue(ui->confFilterV->value());
                emit setConfidenceFilter(ui->confFilterB->isChecked(), ui->confFilterV->value());
            }

            if(obj == ui->timeFilterV)
            {
                ui->timeFilterV->setValue(ui->timeFilterV->value());
                emit setTimeFilter(ui->timeFilterB->isChecked(), ui->timeFilterV->value());
            }

            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

void DeviceArgSetForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // 刷新通过Qt Designer设计的UI部分的翻译
        ui->retranslateUi(this);

        // 然后手动更新所有动态创建的组件的文本
    }

    QWidget::changeEvent(event); // 调用基类处理
}

void DeviceArgSetForm::on_pushButton_clicked()
{
    QToolTip::showText(
        this->cursor().pos(),
        ui->pushButton->toolTip(),
        ui->pushButton,
        ui->pushButton->rect()
        );
}

