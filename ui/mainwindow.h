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
#include <functional>
class QLabel;

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
    void on_pushOfL_D_clicked();
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
    void refreshDataSourceTree();
    void refreshDataSourceTreeDeferred();
    void hideDataWorkspacePanels();
    void fillVariableEditorFromRow(int row);
    void registerTouchInput(QWidget *editor);
    void showTouchInputPopup(const QString &title, const QString &value, const std::function<void (const QString &)> &apply);
    void prepareImeForTransientEditor();

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
    QSet<QWidget*> m_touchInputs;
    QWidget *m_touchInputPanel = nullptr;
    QLineEdit *m_touchInputEdit = nullptr;
    QLabel *m_touchInputTitle = nullptr;
    std::function<void (const QString&)> m_touchInputApply;

};



#endif // MAINWINDOW_H
