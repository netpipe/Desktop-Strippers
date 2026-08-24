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
#include <qprocess.h>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
//
//#define player //starts in

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
#ifdef player
        connect(m_animationTimer, &QTimer::timeout, this, &DesktopDancer::nextFrame); // plays all the frames
#endif
        if (!folderPath.isEmpty() && QDir(folderPath).exists()) {
            loadPngSequence(folderPath);
        } else {
            // Trigger selection safely on application launch
            QTimer::singleShot(100, this, &DesktopDancer::selectFolder);
        }


        //new
        m_opacityEffect = new QGraphicsOpacityEffect(this);
        this->setGraphicsEffect(m_opacityEffect);
        m_fadeAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
        m_fadeAnimation->setDuration(150); // Fast fade

        QTimer *timer = new QTimer(this);
        #ifndef player
        connect(timer, &QTimer::timeout, this, &DesktopDancer::updateFrameByCpu);
        timer->start(1500); // Check every 500ms
        #endif
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
        m_animationTimer->start(100);
    }

    void selectFolder() {
        #ifndef player
        QString dirPath = QApplication::applicationDirPath() + "/hb01"; //QFileDialog::getExistingDirectory(this, "Select Character Sequence Folder", "");
        #else
        QString dirPath =QFileDialog::getExistingDirectory(this, "Select Character Sequence Folder", QApplication::applicationDirPath());
        #endif
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

    void updateFrameByCpu() {
        // 1. Get CPU Temp (Example using a placeholder for smctemp)
        QProcess proc;
#ifdef __APPLE__
        proc.start( QApplication::applicationDirPath() + "/smctemp", QStringList() << "-c"); // Example smctemp call
#else
           proc.start( QApplication::applicationDirPath() + "/test", QStringList() << "-c"); // Example smctemp call
#endif
        proc.waitForFinished();
        double temp = proc.readAllStandardOutput().toDouble();

        // 2. Map Temp to Image Index (Assuming e.g., 30-90°C range)
        int index = (int)((temp - 60) / 60 * m_frameFiles.size());
        index = qBound(0, index, m_frameFiles.size() - 1);

        if (index != m_currentFrame) {
            // 3. Trigger Smooth Transition
            m_targetFrame = index;
            m_fadeAnimation->setStartValue(m_opacityEffect->opacity());
            m_fadeAnimation->setEndValue(0.0);
            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, &DesktopDancer::changeImage);
            m_fadeAnimation->start();
        }
    }

    void changeImage() {
        disconnect(m_fadeAnimation, &QPropertyAnimation::finished, this, &DesktopDancer::changeImage);
        // Load image, then fade in
        setPixmap(QPixmap(m_frameFiles[m_targetFrame].absoluteFilePath()));
        m_fadeAnimation->setStartValue(0.0);
        m_fadeAnimation->setEndValue(1.0);
        m_fadeAnimation->start();
        m_currentFrame = m_targetFrame;
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

#ifdef __APPLE__
   QString getMacCpuTemperature() {
        QProcess process; // Using smctemp as an example so it runs without requiring sudo
        process.start("smctemp", QStringList() << "-c");
        if(process.waitForFinished()) {
            QString output = process.readAllStandardOutput().trimmed();
            qDebug() << "CPU Temperature Output:" << output;
            return output;
        }         else { qDebug() << "Failed to execute temperature command.";
        }
	}

    void getMacCpuUsage() {
        QProcess process;
        process.start("sysctl", QStringList() << "vm.loadavg");

        if (process.waitForFinished()) {
            QString output = process.readAllStandardOutput().trimmed();
            // Output looks like: vm.loadavg: 2.15 1.98 2.03 (1, 5, and 15 minute averages)
            qDebug() << "CPU Load Average:" << output;
        }
    }
#endif

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
    QGraphicsOpacityEffect *m_opacityEffect;
    QPropertyAnimation *m_fadeAnimation;
   // int m_currentFrame;
    int m_targetFrame;
   // QFileInfoList m_frameFiles;
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
