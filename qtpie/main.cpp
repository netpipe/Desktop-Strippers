#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QCursor>
#include <QUrl>
#include <QStringList>
#include <qDebug>
class DesktopDancer : public QLabel {
    Q_OBJECT

public:
    DesktopDancer(QString folderPath = "", QWidget *parent = nullptr) : QLabel(parent) {
        // 1. Native macOS desktop overlay framing settings
     //   setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow);
         setWindowFlags(Qt::FramelessWindowHint );
        setAttribute(Qt::WA_TranslucentBackground); // True alpha PNG rendering channel
        setAlignment(Qt::AlignCenter);
        setAcceptDrops(true);

        // 2. Setup Sequential PNG Core Engine
        m_currentFrame = 0;
        m_animationTimer = new QTimer(this);
        connect(m_animationTimer, &QTimer::timeout, this, &DesktopDancer::nextFrame);

        if (!folderPath.isEmpty() && QDir(folderPath).exists()) {
            loadPngSequence(folderPath);
        } else {
            // Trigger selection safely on application launch
            QTimer::singleShot(100, this, &DesktopDancer::selectFolder);
        }
    }

public slots:
    void loadPngSequence(const QString &dirPath) {
        m_animationTimer->stop();
        m_frameFiles.clear();
        m_currentFrame = 0;

        QDir directory(dirPath);
        // Look for common web sequence styles (png, png alpha formats)
        QStringList filters;
        filters << "*.png";

        // Sort natively by sequence numbers (e.g., frame_01, frame_02, etc.)
        m_frameFiles = directory.entryInfoList(filters, QDir::Files, QDir::Name | QDir::LocaleAware);

        if (m_frameFiles.isEmpty()) {
            setText("No PNGs found in folder");
            qDebug() << "no png's found";
            resize(200, 100);
            return;
        }

        // Draw first baseline frame to scale boundaries correctly
        displayFrame(0);

        // Loop framing intervals (33ms ~ 30 FPS. Increase to 41ms/66ms if too fast)
        m_animationTimer->start(41);
    }

    void selectFolder() {
        QString dirPath = QFileDialog::getExistingDirectory(this, "Select Character Sequence Folder", "");
        if (!dirPath.isEmpty()) {
            loadPngSequence(dirPath);
        } else if (m_frameFiles.isEmpty()) {
            qApp->quit(); // Exit if user cancels initial picker setup
        }
    }

private:
    void nextFrame() {
        if (m_frameFiles.isEmpty()) return;
        m_currentFrame = (m_currentFrame + 1) % m_frameFiles.size();
        displayFrame(m_currentFrame);
    }

    void displayFrame(int index) {
        QPixmap pixmap(m_frameFiles[index].absoluteFilePath());
        if (pixmap.isNull()) return;

        // Automatically scale image bounds if asset layout is too vertical
        if (pixmap.height() > 350) {
            pixmap = pixmap.scaledToHeight(350, Qt::SmoothTransformation);
        }

        setPixmap(pixmap);
        resize(pixmap.size());
    }

protected:
    // --- Mouse Drag Interaction ---
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons() & Qt::LeftButton) {
            move(event->globalPos() - m_dragPosition);
            event->accept();
        }
    }

    // --- Right-Click System Context ---
    void contextMenuEvent(QContextMenuEvent *event) override {
        QMenu menu(this);
        QAction *changeAction = new QAction("Load Character Folder...", this);
        connect(changeAction, &QAction::triggered, this, &DesktopDancer::selectFolder);

        QAction *exitAction = new QAction("Close Player", this);
        connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

        menu.addAction(changeAction);
        menu.addSeparator();
        menu.addAction(exitAction);

        menu.exec(event->globalPos());
    }

    // --- Folder Drag and Drop Mechanics ---
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        }
    }

    void dropEvent(QDropEvent *event) override {
        const QMimeData *mimeData = event->mimeData();
        if (mimeData->hasUrls() && !mimeData->urls().isEmpty()) {
            QString localPath = mimeData->urls().first().toLocalFile();
            if (QFileInfo(localPath).isDir()) {
                loadPngSequence(localPath);
                event->acceptProposedAction();
            }
        }
    }

private:
    QFileInfoList m_frameFiles;
    int m_currentFrame;
    QTimer *m_animationTimer;
    QPoint m_dragPosition;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString initialDir = "";
    if (argc > 1) {
        initialDir = QString::fromLocal8Bit(argv[1]);
    }

    DesktopDancer dancer(initialDir);
    dancer.show();

    return app.exec();
}

#include "main.moc"
