#include "update_dialog.h"

#include "ui_update_dialog.h"

namespace dblsqd
{

/*!
 * \class UpdateDialog
 * \brief A dialog class for displaying and downloading update information.
 *
 * UpdateDialog is a drop-in class for adding a fully-functional auto-update
 * component to an existing application.
 *
 * The most simple integration is
 * possible with just three lines of code:
 * \code
 * dblsqd::Feed* feed = new dblsqd::Feed();
 * feed->setUrl("https://feeds.dblsqd.com/:app_token");
 * dblsqd::UpdateDialog* updateDialog = new dblsqd::UpdateDialog(feed);
 * \endcode
 *
 * The update dialog can also display an application icon which can be set with
 * setIcon().
 */

/*!
 * \enum UpdateDialog::Type
 * \brief This flag determines the if and when the UpdateDialog is displayed
 * automatically.
 *
 * *OnUpdateAvailable*: Automatically display the dialog as soon as the Feed
 * has been downloaded and parsed and if there is a newer version than the
 * current version returned by QCoreApplication::applicationVersion().
 *
 * *OnLastWindowClosed*: If there is a newer version available than the current
 * version returned by QCoreApplication::applicationVersion(), the update
 * dialog is displayed when QGuiApplication emits the lastWindowClosed() event.
 * Note that when this flag is used,
 * QGuiApplication::setQuitOnLastWindowClosed(false) will be called.
 *
 * *Manual*: The dialog is only displayed when explicitly requested via show()
 * or exec().
 * Note that update information might not be available instantly after
 * constructing an UpdateDialog.
 *
 * *ManualChangelog*: The dialog is only displayed when explicitly requested via
 * show() or exec().
 * Instead of the full update interface, only the changelog will be shown.
 */

/*!
 * \brief Constructs a new UpdateDialog.
 *
 * A Feed object needs to be constructed first and passed to this constructor.
 * Feed::load() does not need to be called on the Feed object.
 *
 * The given UpdateDialog::Type flag determines when/if the dialog is shown
 * automatically.
 *
 * UpdateDialog uses QSettings to save information such as when a release was
 * skipped by the users. If you want to use a specially initialized QSettings
 * object, you may also pass it to this constructor.
 *
 */
UpdateDialog::UpdateDialog(Feed* feed, int type, QWidget* parent, QSettings* settings)
    : QDialog(parent)
    , ui(new Ui::UpdateDialog)
    , feed(feed)
    , type(type)
    , settings(settings)
    , accepted(false)
    , isDownloadFinished(false)
    , acceptedInstallButton(NULL)
{
    ui->setupUi(this);

    QPalette palette        = this->palette();
    QString textColor       = palette.color(QPalette::Text).name();
    QString backgroundColor = palette.color(QPalette::Base).name();
    QString labelChangelogStyle =
        QString("color: %1; background: %2").arg(textColor, backgroundColor);
    ui->labelChangelog->setStyleSheet(labelChangelogStyle);

    ui->buttonCancel->addAction(ui->actionCancel);
    ui->buttonCancel->addAction(ui->actionSkip);
    ui->buttonCancel->setDefaultAction(ui->actionCancel);

    _openExternalLinks = true;
    connect(ui->labelChangelog, SIGNAL(linkActivated(QString)), this,
            SLOT(onLinkActivated(QString)));

    switch (type) {
    case OnUpdateAvailable: {
        connect(this, SIGNAL(ready()), this, SLOT(showIfUpdatesAvailable()));
        break;
    }
    case OnLastWindowClosed: {
        QGuiApplication* app = (QGuiApplication*)QApplication::instance();
        app->setQuitOnLastWindowClosed(false);
        connect(app, SIGNAL(lastWindowClosed()), this,
                SLOT(showIfUpdatesAvailableOrQuit()));
        break;
    }
    case Manual: {
        // don’t do anything
    }
    }

    if (feed->isReady()) {
        handleFeedReady();
    } else {
        setupLoadingUi();
        feed->load();
        connect(feed, SIGNAL(ready()), this, SLOT(handleFeedReady()));
    }
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

/*
 * Setters
 */
/*!
 * \brief Sets the icon displayed in the update window.
 */
void UpdateDialog::setIcon(QPixmap pixmap)
{
    ui->labelIcon->setPixmap(QPixmap(pixmap));
    ui->labelIcon->setHidden(false);
}

void UpdateDialog::setIcon(QString fileName)
{
    ui->labelIcon->setPixmap(QPixmap(fileName));
    ui->labelIcon->setHidden(false);
}

/*!
 * \brief Sets the minimum version to be displayed in the changelog.
 * Defaults to QApplication::applicationVersion() if not set.
 * \param version
 */
void UpdateDialog::setMinVersion(QString version)
{
    _minVersion = version;
    setupChangelogUi();
}

/*!
 * \brief Sets the maximum version to be displayed in the changelog
 * \param version
 */
void UpdateDialog::setMaxVersion(QString version)
{
    _maxVersion = version;
    setupChangelogUi();
}

/*!
 * \brief Convenience method for setting minimum and maximum version to be displayed in
 * the changelog. maximumVersion is set to QApplication::applicationVersion()
 *
 * \param previousVersion
 */
void UpdateDialog::setPreviousVersion(QString previousVersion)
{
    _previousVersion = previousVersion;
    _minVersion      = previousVersion;
    _maxVersion      = QApplication::applicationVersion();
    setupChangelogUi();
}

/*!
 * \brief Adds a custom button for handling update installation.
 * \param button
 *
 * When the custom button is clicked after an update has been downloaded or when
 * downloading an update that was started by clicking the button has finished,
 * installButtonClicked(QAbstractButton* button, QString filePath) is emitted.
 */
void UpdateDialog::addInstallButton(QAbstractButton* button)
{
    installButtons.append(button);
    ui->buttonContainer->layout()->addWidget(button);
    if (isVisible() && ui->buttonCancel->isVisible()) {
        setupUpdateUi();
    }
}

/*!
 * \propget UpdateExternalLinks
 *
 * Determines if links in the changelog should be opened automatically by
 * QDesktopServices::openUrl() when a user clicks on them.
 * If set to false, the linkActivated() signal is emitted instead.
 *
 * The default value is true.

 */
bool UpdateDialog::openExternalLinks()
{
    return _openExternalLinks;
}

/*!
 * \propset UpdateDialog::openExternalLinks
 */
void UpdateDialog::setOpenExternalLinks(bool open)
{
    _openExternalLinks = open;
}

/*
 * Public Slots
 */
/*!
 * \brief Default handler for the install button.
 *
 * Closes the dialog if no other action (such as
 * downloading or installing a Release) is required first.
 */
void UpdateDialog::onButtonInstall()
{
    accepted = true;
    if (isDownloadFinished) {
        startUpdate();
    } else if (!latestRelease.getVersion().isEmpty()) {
        startDownload();
    } else {
        done(QDialog::Accepted);
    }
}

void UpdateDialog::onButtonCustomInstall()
{
    accepted = true;
    if (isDownloadFinished) {
        emit installButtonClicked((QAbstractButton*)sender(), updateFilePath);
    } else if (!latestRelease.getVersion().isEmpty()) {
        acceptedInstallButton = (QAbstractButton*)sender();
        startDownload();
    } else {
        done(QDialog::Accepted);
    }
}

/*!
 * \brief Skips the latest retrieved Release.
 *
 * If a release has been skipped, UpdateDialog will not be displayed
 * automatically when using Type::OnUpdateAvailable or
 * Type::OnLastWindowClosed.
 */
void UpdateDialog::skip()
{
    if (!updateFilePath.isEmpty()) {
        QFile::remove(updateFilePath);
    }
    setSettingsValue("skipRelease", latestRelease.getVersion(), settings);
    done(QDialog::Rejected);
}

/*!
 * \brief Shows the dialog if there are available updates.
 */
void UpdateDialog::showIfUpdatesAvailable()
{
    QString latestVersion = latestRelease.getVersion();
    bool skipRelease =
        (settingsValue("skipRelease", "", settings).toString() == latestVersion);
    if (!latestVersion.isEmpty() && !skipRelease) {
        show();
    }
}

/*!
 * \brief Shows the dialog if there are updates available or quits the application.
 */
void UpdateDialog::showIfUpdatesAvailableOrQuit()
{
    if (type == OnLastWindowClosed) {
        QGuiApplication* app = (QGuiApplication*)QApplication::instance();
        app->setQuitOnLastWindowClosed(true);
        disconnect(app, SIGNAL(lastWindowClosed()), this,
                   SLOT(showIfUpdatesAvailableOrQuit()));
    }
    QString latestVersion = latestRelease.getVersion();
    bool skipRelease =
        (settingsValue("skipRelease", "", settings).toString() == latestVersion);
    if (!latestVersion.isEmpty() && !skipRelease) {
        show();
    } else {
        QCoreApplication::quit();
    }
}

/*
 * Static settings helpers
 */
QVariant UpdateDialog::settingsValue(QString key, QVariant defaultValue,
                                     QSettings* settings)
{
    return settings->value("DBLSQD/" + key, defaultValue);
}

void UpdateDialog::setSettingsValue(QString key, QVariant value, QSettings* settings)
{
    settings->setValue("DBLSQD/" + key, value);
}

void UpdateDialog::removeSetting(QString key, QSettings* settings)
{
    settings->remove("DBLSQD/" + key);
}

void UpdateDialog::setDefaultSettingsValue(QString key, QVariant value,
                                           QSettings* settings)
{
    if (settings->contains("DBLSQD/" + key))
        return;
    setSettingsValue(key, value, settings);
}

/*!
 * \brief Enables or disables automatic downloads.
 */
void UpdateDialog::enableAutoDownload(bool enabled, QSettings* settings)
{
    setSettingsValue("autoDownload", enabled, settings);
}

/*!
 * \brief Returns true if automatic downloads are enabled.
 *
 * If defaultValue is provided, it is stored if no other value has previously been set.
 */
bool UpdateDialog::autoDownloadEnabled(QVariant defaultValue, QSettings* settings)
{
    if (defaultValue.isValid()) {
        setDefaultSettingsValue("autoDownload", defaultValue, settings);
    } else {
        defaultValue = false;
    }
    return settingsValue("autoDownload", defaultValue, settings).toBool();
}

/*!
 * \overload
 */
bool UpdateDialog::autoDownloadEnabled(QSettings* settings)
{
    return settingsValue("autoDownload", false, settings).toBool();
}

/*
 * Helpers
 */

void UpdateDialog::adjustDialogSize()
{
    adjustSize();

/*HACK: Qt seems to incorrectly calculate window geometry on Windows.
        This code avoids warning messages logged by the application
        in that case.*/
#if defined(Q_OS_WIN) || defined(Q_WS_WIN)
    QSize dialogSize = size();
    resize(dialogSize.width(), dialogSize.height() + 3);
#endif
}

void UpdateDialog::resetUi()
{
    QList<QWidget*> hiddenWidgets;
    for (int i = 0; i < installButtons.size(); i++) {
        hiddenWidgets << installButtons.at(i);
    }
    hiddenWidgets << ui->headerContainer << ui->labelIcon << ui->headerContainerLoading
                  << ui->headerContainerNoUpdates << ui->headerContainerChangelog
                  << ui->scrollAreaChangelog << ui->progressBar << ui->checkAutoDownload
                  << ui->buttonCancel << ui->buttonCancelLoading << ui->buttonConfirm
                  << ui->buttonInstall;
    for (int i = 0; i < hiddenWidgets.size(); i++) {
        hiddenWidgets.at(i)->hide();
        hiddenWidgets.at(i)->disconnect();
    }
    ui->progressBar->reset();
    adjustDialogSize();
}

void UpdateDialog::setupLoadingUi()
{
    resetUi();
    ui->headerContainerLoading->show();
    ui->progressBar->show();
    ui->progressBar->setMaximum(0);
    ui->progressBar->setMinimum(0);
    ui->buttonCancelLoading->show();
    ui->buttonCancelLoading->setFocus();
    connect(ui->buttonCancelLoading, SIGNAL(clicked(bool)), this, SLOT(reject()));
    adjustDialogSize();
}

void UpdateDialog::setupUpdateUi()
{
    resetUi();

    QList<QWidget*> showWidgets;
    showWidgets << ui->headerContainer << ui->scrollAreaChangelog << ui->checkAutoDownload
                << ui->buttonCancel << ui->buttonInstall;
    for (int i = 0; i < showWidgets.size(); i++) {
        showWidgets.at(i)->show();
    }

    QList<QLabel*> labels;
    labels << ui->labelHeadline << ui->labelInfo;
    for (int i = 0; i < labels.size(); i++) {
        QString text = labels.at(i)->text();
        replaceAppVars(text);
        labels.at(i)->setText(text);
    }
    ui->labelChangelog->setText(generateChangelogDocument());

    ui->checkAutoDownload->setChecked(autoDownloadEnabled(settings));

    // Adapt buttons if release has been downloaded already
    if (isDownloadFinished) {
        ui->progressBar->show();
        ui->progressBar->setMaximum(1);
        ui->progressBar->setValue(1);
    }

    connect(feed, SIGNAL(downloadFinished()), this, SLOT(handleDownloadFinished()));
    connect(feed, SIGNAL(downloadError(QString)), this,
            SLOT(handleDownloadError(QString)));
    connect(feed, SIGNAL(downloadProgress(qint64, qint64)), this,
            SLOT(updateProgressBar(qint64, qint64)));

    connect(ui->buttonConfirm, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->actionCancel, SIGNAL(triggered()), this, SLOT(reject()));
    connect(ui->actionSkip, SIGNAL(triggered()), this, SLOT(skip()));
    connect(ui->checkAutoDownload, SIGNAL(toggled(bool)), this,
            SLOT(autoDownloadCheckboxToggled(bool)));

    // Install buttons
    if (installButtons.isEmpty()) {
        ui->buttonInstall->setFocus();
        connect(ui->buttonInstall, SIGNAL(clicked()), this, SLOT(onButtonInstall()));
    } else {
        ui->buttonInstall->hide();
        for (int i = 0; i < installButtons.size(); i++) {
            installButtons.at(i)->show();
            connect(installButtons.at(i), SIGNAL(clicked(bool)), this,
                    SLOT(onButtonCustomInstall()));
        }
        installButtons.last()->setFocus();
    }

    adjustDialogSize();
}

void UpdateDialog::setupChangelogUi()
{
    resetUi();

    QList<QWidget*> showWidgets;
    showWidgets << ui->headerContainerChangelog << ui->buttonConfirm
                << ui->scrollAreaChangelog;
    for (int i = 0; i < showWidgets.size(); i++) {
        showWidgets.at(i)->show();
    }
    QList<QLabel*> labels;
    labels << ui->labelHeadlineChangelog << ui->labelInfoChangelog;
    for (int i = 0; i < labels.size(); i++) {
        QString text = labels.at(i)->text();
        replaceAppVars(text);
        labels.at(i)->setText(text);
    }
    ui->labelChangelog->setText(generateChangelogDocument());
    connect(ui->buttonConfirm, SIGNAL(clicked(bool)), this, SLOT(accept()));
    ui->buttonConfirm->setFocus();
    adjustDialogSize();
}

void UpdateDialog::setupNoUpdatesUi()
{
    resetUi();
    QList<QWidget*> showWidgets;
    showWidgets << ui->headerContainerNoUpdates << ui->buttonConfirm;
    for (int i = 0; i < showWidgets.size(); i++) {
        showWidgets.at(i)->show();
    }
    ui->buttonConfirm->setFocus();

    QString text = ui->labelHeadlineNoUpdates->text();
    replaceAppVars(text);
    ui->labelHeadlineNoUpdates->setText(text);

    connect(ui->buttonConfirm, SIGNAL(clicked(bool)), this, SLOT(accept()));
    adjustDialogSize();
}

void UpdateDialog::disableButtons(bool disable)
{
    QList<QWidget*> buttons;
    for (int i = 0; i < installButtons.size(); i++) {
        buttons << installButtons.at(i);
    }
    buttons << ui->buttonCancel << ui->buttonCancelLoading << ui->buttonConfirm
            << ui->buttonConfirm << ui->buttonInstall << ui->checkAutoDownload;
    for (int i = 0; i < buttons.size(); i++) {
        buttons.at(i)->setDisabled(disable);
    }
}

void UpdateDialog::replaceAppVars(QString& string)
{
    string.replace("%APPNAME%", QCoreApplication::applicationName());
    string.replace("%CURRENT_VERSION%", QCoreApplication::applicationVersion());
    string.replace("%UPDATE_VERSION%", latestRelease.getVersion());
}

QString UpdateDialog::generateChangelogDocument()
{
    QString changelog;
    QList<Release> changelogReleases;
    if (_minVersion.isEmpty() && _maxVersion.isEmpty()) {
        changelogReleases = updates;
    } else {
        Release minRelease(_minVersion.isEmpty() ? QApplication::applicationVersion()
                                                 : _minVersion);
        Release maxRelease(_maxVersion);
        for (int i = 0; i < releases.size(); i++) {
            if (minRelease < releases.at(i)
                && (_maxVersion.isEmpty() || releases.at(i) <= maxRelease)) {
                changelogReleases << releases.at(i);
            }
        }
    }
    for (int i = 0; i < changelogReleases.size(); i++) {
        QString h2Style = "font-size: medium;";
        if (i > 0) {
            h2Style.append("margin-top: 1em;");
        }
        changelog.append("<h2 style=\"" + h2Style + "\">"
                         + changelogReleases.at(i).getVersion() + "</h2>");
        changelog.append("<p>" + changelogReleases.at(i).getChangelog() + "</p>");
    }
    return changelog;
}

void UpdateDialog::startDownload()
{
    feed->downloadRelease(latestRelease);
    disableButtons(true);
}

void UpdateDialog::startUpdate()
{
    if (QDesktopServices::openUrl(QUrl::fromLocalFile(updateFilePath))) {
        done(QDialog::Accepted);
        QApplication::quit();
    } else {
        handleDownloadError(tr("Could not open downloaded file %1").arg(updateFilePath));
    }
}

/*
 * Private Slots
 */

void UpdateDialog::autoDownloadCheckboxToggled(bool enabled)
{
    enableAutoDownload(enabled, settings);
}

void UpdateDialog::handleFeedReady()
{
    // Retrieve update information
    Release currentRelease(QApplication::applicationVersion());
    updates  = feed->getUpdates(currentRelease);
    releases = feed->getReleases();
    if (!updates.isEmpty()) {
        latestRelease = updates.first();
    }

    if (type == ManualChangelog) {
        setupChangelogUi();
        emit ready();
        return;
    }

    // Check if an update has been downloaded previously
    updateFilePath = settingsValue("updateFilePath", "", settings).toString();
    if (!updateFilePath.isEmpty() && QFile::exists(updateFilePath)) {
        QString updateFileVersion =
            settingsValue("updateFileVersion", "", settings).toString();
        if (updateFileVersion != latestRelease.getVersion()
            || updateFileVersion == QApplication::applicationVersion()) {
            QFile::remove(updateFilePath);
            removeSetting("updateFilePath");
            removeSetting("updateFileVersion");
            updateFilePath = "";
        } else {
            isDownloadFinished = true;
        }
    }

    // Check if there are any updates
    if (updates.isEmpty()) {
        setupNoUpdatesUi();
        return;
    }

    // Automatic downloads
    QString latestVersion = latestRelease.getVersion();
    bool skipRelease =
        (settingsValue("skipRelease", "", settings).toString() == latestVersion);
    bool autoDownload = autoDownloadEnabled(settings) && (!skipRelease);
    if (autoDownload && !isDownloadFinished) {
        startDownload();
    }

    // Setup UI
    setupUpdateUi();
    emit ready();
}

void UpdateDialog::handleDownloadFinished()
{
    QTemporaryFile* file = feed->getDownloadFile();
    isDownloadFinished   = true;
    updateFilePath       = file->fileName();
    file->setAutoRemove(false);
    file->close();
    file->flush();  // Ensure all data is written to disk
    file->deleteLater();
    setSettingsValue("updateFilePath", updateFilePath, settings);
    setSettingsValue("updateFileVersion", latestRelease.getVersion(), settings);

    if (accepted) {
        if (acceptedInstallButton == NULL) {
            // Use QTimer::singleShot to ensure file is fully closed before opening
            QTimer::singleShot(100, this, SLOT(startUpdate()));
        } else {
            emit installButtonClicked(acceptedInstallButton, updateFilePath);
        }

    } else {
        disableButtons(false);
    }
}

void UpdateDialog::handleDownloadError(QString message)
{
    QMessageBox* messageBox = new QMessageBox(this);
    messageBox->setIcon(QMessageBox::Warning);
    messageBox->setText("There was an error while downloading the update.");
    messageBox->setInformativeText(message);
    messageBox->show();
    done(QDialog::Rejected);
}

void UpdateDialog::updateProgressBar(qint64 bytesReceived, qint64 bytesTotal)
{
    ui->progressBar->show();
    ui->progressBar->setMaximum(bytesTotal / 1024);
    ui->progressBar->setValue(bytesReceived / 1024);
}

void UpdateDialog::onLinkActivated(QString link)
{
    if (_openExternalLinks) {
        QDesktopServices::openUrl(link);
    } else {
        emit linkActivated(link);
    }
}

/*
 * Signals
 */
/*! \fn void Feed::ready()
 * This signal is emitted when a updates are available and the UpdateDialog is
 * ready to be shown with show() or exec().
 */

/*! \fn void Feed::installButtonClicked(QAbstractButton* button, QString filePath)
 * This signal is emitted when a custom install button was clicked.
 */

}  // namespace dblsqd
