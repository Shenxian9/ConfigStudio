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
#include <QDialog>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QQuickWidget>
#include <QFrame>
#include <QPointer>
#include <QStandardItemModel>

#include "componentpalette.h"
#include "palettebinder.h"
#include "canvasitem.h"
#include "canvasview.h"
#include "fullscreenview.h"
#include "runtimewindow.h"
#include "variablemodel.h"
#include "runtimesimulator.h"
#include "serialdatasource.h"
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
    void refreshDataSourceTree();
    void refreshDataSourceTreeDeferred();
    void showSerialConfigDialog();
    void showMappingDialog();
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
    SerialVariableMapper *m_serialMapper = nullptr;
    QStandardItemModel *m_dataSourceTreeModel = nullptr;
    QPointer<QDialog> m_serialConfigDialog;
    QPointer<QDialog> m_mappingDialog;

};



#endif // MAINWINDOW_H
