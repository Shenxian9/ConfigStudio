#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QDebug>
#include <QLayout>
#include <QTimer>
#include <QGuiApplication>
#include <QInputMethod>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QQuickWidget>
#include <QFrame>
#include <QPointer>
#include <QStandardItemModel>
#include <QSet>
#include <QHash>
#include <QVector>
#include <functional>
class QLabel;
class QListWidget;
class QListWidgetItem;

#include "componentpalette.h"
#include "palettebinder.h"
#include "canvasitem.h"
#include "canvasview.h"
#include "fullscreenview.h"
#include "runtimewindow.h"
#include "variablemodel.h"
#include "runtimesimulator.h"
#include "serialdatasource.h"
#include "modbusrtudatasource.h"
#include "virtualkeyboardhost.h"
#include "optioncyclebutton.h"
#include "projectfilemanager.h"
#include "projectstoragemanager.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    // void MainWindow::onItemSelected(CanvasItem *item);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent*) override;
    void setupIconButton(QPushButton* btn, const QString& iconPath);
    bool eventFilter(QObject *watched, QEvent *event) override;


private slots:
    void onCanvasEmptyClicked();

    void onItemSelected(CanvasItem *item);


    void on_deleteButton_clicked();


    void on_buttonOfFullscreen_clicked();






    void on_pushOfDatasrc_clicked();

    void on_pushOfDesign_clicked();
    void on_pushOfExit_clicked();
    void on_pushOfL_D_clicked();
    void on_pushOfSave_clicked();
    void on_pushOfLoad_clicked();
    void onProjectPanelPrimaryClicked();
    void onProjectPanelSecondaryClicked();
    void onProjectPanelDeleteClicked();
    void onProjectPanelCancelClicked();
    void onProjectStoragePathChanged(const QString &path);
    void showSerialConfigDialog();
    void applySerialConfigFromPanel();
    void showAddVariableDialog();
    void showEditVariableDialog();
    void deleteSelectedVariable();
    void applyVariableFromPanel();
    void updateVariableActionButtons();
    void applyDataSourceMode();

public:
    SerialDataSource *serialDataSourceForTest() const { return m_serialDataSource; }
    VariableModel *variableModelForTest() const { return m_variableModel; }
    ModbusRtuDataSource *modbusDataSourceForTest() const { return m_modbusDataSource; }
    RuntimeSimulator *runtimeSimulatorForTest() const { return m_runtimeSimulator; }

private:
    void setupDataWorkspace();
    void setupDataWorkspacePanels();
    int selectedDataSourceRow() const;
    void updateDataSourceActionButtons();
    void refreshDataSourceTree();
    void refreshDataSourceTreeDeferred();
    void ensureErrorNoticePanel();
    void showErrorNotice(const QString &title, const QString &message);
    void hideDataWorkspacePanels();
    void fillVariableEditorFromRow(int row);
    void registerTouchInput(QWidget *editor);
    void showTouchInputPopup(const QString &title, const QString &value, const std::function<void (const QString &)> &apply);
    void prepareImeForTransientEditor();
    void updateVariableViewColumns();
    void updateModeLabel();
    void appendConnectionLog(const QString &message);
    void ensureExitConfirmPanel();
    void performSafeExit();
    ProjectData buildProjectDataSnapshot();
    bool restoreProjectData(const ProjectData &project, QString *errorText);
    void clearCurrentProjectState();
    QString ensureCanvasItemId(CanvasItem *item);
    void applySerialConfigToPanels(const SerialPortConfig &cfg);
    void ensureProjectPanel();
    void ensureConfirmPanel();
    void showProjectPanel(bool saveMode);
    void hideProjectPanel();
    void refreshProjectPanelList();
    ProjectFileInfo selectedProjectInfo() const;
    void setProjectPanelStatus(const QString &message, bool isError);
    void applyProjectStorageRoot(const QString &rootPath);
    bool saveProjectToPath(const QString &path, const QString &projectName);
    bool loadProjectFromPath(const QString &path);
    void showConfirmPanel(const QString &title, const QString &message, const std::function<void ()> &onConfirm);
    QString displayFileSize(qint64 bytes) const;

    void showProperties(CanvasItem *item);
    void clearProperties();
    void onPropertyChanged(int row, int col);
    void enterFullScreenMode();
    void exitFullScreenMode();
    void exitRuntime(RuntimeWindow *rt);
    Ui::MainWindow *ui;

    CanvasItem *m_currentItem = nullptr;

    CanvasView *m_canvas;

    QRect m_canvasOriginalGeometry;
    RuntimeWindow *m_runtimeWindow = nullptr;

    QLayout* m_canvasOriginalLayout = nullptr;
    int m_canvasOriginalIndex = -1;
    QSize m_canvasOriginalSize;
    QWidget *m_canvasOriginalParent = nullptr;

    QList<int> m_originalStretchList;


    void editPropertyCell(int row, int col);

    void applyCanvasTheme(bool darkMode);
    void enforceCanvasFrameRatio();
    void refreshActionButtonIcons();

    VariableModel* m_variableModel;
    DataBindingManager* m_bindingMgr;

    QQuickWidget *m_keyboard = nullptr;

    QWidget *m_propertyInputPanel = nullptr;
    QLineEdit *m_propertyInputEdit = nullptr;
    QPointer<CanvasItem> m_pendingPropertyItem;
    QString m_pendingPropertyKey;
    int m_pendingPropertyRow = -1;

    bool m_darkCanvasMode = false;

    SerialDataSource *m_serialDataSource = nullptr;
    ModbusRtuDataSource *m_modbusDataSource = nullptr;
    QStandardItemModel *m_dataSourceTreeModel = nullptr;
    RuntimeSimulator *m_runtimeSimulator = nullptr;
    QPushButton *m_dataSourceModeCombo = nullptr;
    QString m_lastCommStatus;
    QSet<QString> m_loggedFirstReadVarIds;
    QVector<SerialPortConfig> m_modbusConfigs;
    int m_editingDataSourceRow = -1;

    QWidget *m_serialConfigPanel = nullptr;
    QLineEdit *m_serialPortEdit = nullptr;
    QLineEdit *m_serialDeviceEdit = nullptr;
    OptionCycleButton *m_serialBaudCombo = nullptr;
    OptionCycleButton *m_dataBitsCombo = nullptr;
    OptionCycleButton *m_parityCombo = nullptr;
    OptionCycleButton *m_stopBitsCombo = nullptr;
    QSpinBox *m_slaveIdSpin = nullptr;
    QSpinBox *m_timeoutSpin = nullptr;
    QSpinBox *m_retrySpin = nullptr;
    QSpinBox *m_pollIntervalSpin = nullptr;
    OptionCycleButton *m_functionCodeCombo = nullptr;

    QWidget *m_variableEditorPanel = nullptr;
    QLineEdit *m_variableIdEdit = nullptr;
    QLineEdit *m_variableNameEdit = nullptr;
    QLineEdit *m_variableDeviceEdit = nullptr;
    OptionCycleButton *m_variableTypeCombo = nullptr;
    OptionCycleButton *m_variableAreaCombo = nullptr;
    QSpinBox *m_variableAddressSpin = nullptr;
    QSpinBox *m_variableCountSpin = nullptr;
    QSpinBox *m_variableBitOffsetSpin = nullptr;
    QLineEdit *m_variableUnitEdit = nullptr;
    QDoubleSpinBox *m_variableScaleSpin = nullptr;
    QCheckBox *m_variableReadOnlyCheck = nullptr;
    OptionCycleButton *m_variableEndianCombo = nullptr;
    int m_variableEditorRow = -1;
    QFrame *m_errorNoticePanel = nullptr;
    QLabel *m_errorNoticeTitleLabel = nullptr;
    QLabel *m_errorNoticeMessageLabel = nullptr;
    QSet<QWidget*> m_touchInputs;
    QHash<QObject*, QWidget*> m_touchInputTargets;
    QWidget *m_touchInputPanel = nullptr;
    QLineEdit *m_touchInputEdit = nullptr;
    std::function<void (const QString&)> m_touchInputApply;
    QFrame *m_exitConfirmPanel = nullptr;
    QLabel *m_exitConfirmLabel = nullptr;

    ProjectStorageManager *m_projectStorage = nullptr;
    QString m_currentProjectFilePath;
    QString m_currentProjectName;
    bool m_projectDirty = false;

    QFrame *m_projectPanel = nullptr;
    QLabel *m_projectPanelTitle = nullptr;
    QLabel *m_projectPanelHint = nullptr;
    QLabel *m_projectPanelStatus = nullptr;
    OptionCycleButton *m_projectStoragePathCombo = nullptr;
    QLineEdit *m_projectNameEdit = nullptr;
    QListWidget *m_projectListWidget = nullptr;
    QPushButton *m_projectPrimaryBtn = nullptr;
    QPushButton *m_projectSecondaryBtn = nullptr;
    QPushButton *m_projectDeleteBtn = nullptr;
    QPushButton *m_projectCancelBtn = nullptr;
    bool m_projectPanelSaveMode = true;

    QFrame *m_confirmPanel = nullptr;
    QLabel *m_confirmPanelTitle = nullptr;
    QLabel *m_confirmPanelMessage = nullptr;
    std::function<void()> m_confirmAcceptAction;

};



#endif // MAINWINDOW_H
