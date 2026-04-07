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
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QQuickWidget>
#include <QFrame>
#include <QPointer>
#include <QStandardItemModel>
#include <QListWidget>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QTextEdit>

#include "componentpalette.h"
#include "palettebinder.h"
#include "canvasitem.h"
#include "canvasview.h"
#include "fullscreenview.h"
#include "runtimewindow.h"
#include "variablemodel.h"
#include "runtimesimulator.h"
#include "serialdatasource.h"
#include "modbusmappingdefs.h"
#include "virtualkeyboardhost.h"
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


private slots:
    void onCanvasEmptyClicked();

    void onItemSelected(CanvasItem *item);


    void on_deleteButton_clicked();


    void on_buttonOfFullscreen_clicked();





    void on_pushOfDatasrc_clicked();

    void on_pushOfDesign_clicked();
    void on_pushOfL_D_clicked();

private:
    void setupDataWorkspace();
    void setupDataWorkspacePanels();
    void refreshDataSourceTree();
    void refreshDataSourceTreeDeferred();
    void showSerialConfigDialog();
    void showMappingDialog();
    void hideDataWorkspacePanels();
    void applySerialConfigFromPanel();
    void applyMappingFromPanel();
    void prepareImeForTransientEditor();

    void refreshPollGroupList();
    void refreshPointList();
    void refreshPollGroupCombo();
    void clearPollGroupForm();
    void clearPointForm();
    void loadPollGroupToForm(int index);
    void loadPointToForm(int index);
    PollGroupDefinition buildPollGroupFromUi() const;
    ModbusPointDefinition buildPointFromUi() const;
    void loadPollGroupToUi(const PollGroupDefinition &group);
    void loadPointToUi(const ModbusPointDefinition &point);
    void saveCurrentPollGroup();
    void saveCurrentPoint();
    void updatePointEditorByKind();
    void injectDebugModbusSamples();

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
    SerialVariableMapper *m_serialMapper = nullptr;
    ModbusMappingConfig m_modbusMappingConfig;
    QStandardItemModel *m_dataSourceTreeModel = nullptr;

    QWidget *m_serialConfigPanel = nullptr;
    QLineEdit *m_serialPortEdit = nullptr;
    QComboBox *m_serialBaudCombo = nullptr;
    QComboBox *m_dataBitsCombo = nullptr;
    QComboBox *m_parityCombo = nullptr;
    QComboBox *m_stopBitsCombo = nullptr;
    QSpinBox *m_slaveIdSpin = nullptr;
    QSpinBox *m_timeoutSpin = nullptr;
    QSpinBox *m_retrySpin = nullptr;
    QSpinBox *m_pollIntervalSpin = nullptr;
    QComboBox *m_functionCodeCombo = nullptr;

    QWidget *m_mappingPanel = nullptr;

    QListWidget *m_pollGroupList = nullptr;
    QLineEdit *m_pollGroupIdEdit = nullptr;
    QLineEdit *m_pollGroupNameEdit = nullptr;
    QCheckBox *m_pollGroupEnabledCheck = nullptr;
    QSpinBox *m_pollGroupIntervalSpin = nullptr;
    QSpinBox *m_pollGroupPrioritySpin = nullptr;
    QTextEdit *m_pollGroupDescriptionEdit = nullptr;
    int m_currentPollGroupIndex = -1;

    QListWidget *m_pointList = nullptr;
    QLineEdit *m_pointIdEdit = nullptr;
    QLineEdit *m_pointNameEdit = nullptr;
    QLineEdit *m_pointVarIdEdit = nullptr;
    QCheckBox *m_pointEnabledCheck = nullptr;
    QComboBox *m_pointKindCombo = nullptr;
    QSpinBox *m_pointSlaveIdSpin = nullptr;
    QSpinBox *m_pointFunctionCodeSpin = nullptr;
    QSpinBox *m_pointAddressSpin = nullptr;
    QSpinBox *m_pointQuantitySpin = nullptr;
    QCheckBox *m_pointReadableCheck = nullptr;
    QCheckBox *m_pointWritableCheck = nullptr;
    QComboBox *m_pointDataTypeCombo = nullptr;
    QComboBox *m_pointByteOrderCombo = nullptr;
    QComboBox *m_pointWordOrderCombo = nullptr;
    QDoubleSpinBox *m_pointScaleSpin = nullptr;
    QDoubleSpinBox *m_pointOffsetSpin = nullptr;
    QComboBox *m_pointPollGroupCombo = nullptr;
    QComboBox *m_pointWriteStrategyCombo = nullptr;
    QTextEdit *m_pointDescriptionEdit = nullptr;
    int m_currentPointIndex = -1;
};



#endif // MAINWINDOW_H
