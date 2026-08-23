#include <QApplication>
#include <QLabel>
#include <QMovie>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>

class DesktopDancer : public QLabel {
    Q_OBJECT

public:
    DesktopDancer(QString gifPath = "", QWidget *parent = nullptr) : QLabel(parent) {
        // 1. Setup window attributes for a transparent desktop character overlay
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow);
        setAttribute(Qt::WA_TranslucentBackground); // True alpha channel transparency
        setAlignment(Qt::AlignCenter);
        setAcceptDrops(true);

        // 2. Setup the GIF Animation engine
        m_movie = new QMovie(this);
        m_movie->setCacheMode(QMovie::CacheAll);

        if (!gifPath.isEmpty()) {
            loadGif(gifPath);
        } else {
            // Trigger file dialog safely after window initialization
            QTimer::singleShot(100, this, &DesktopDancer::selectGifFile);
        }
    }

public slots:
    void loadGif(const QString &path) {
        m_movie->stop();
        m_movie->setFileName(path);

        if (!m_movie->isValid()) {
            setText("Invalid GIF File");
            resize(150, 150);
            return;
        }

        m_movie->jumpToFrame(0);
        QSize size = m_movie->currentImage().size();

        // Downscale bounds dynamically if the GIF layout is too tall
        if (size.height() > 300) {
            double scaleRatio = 300.0 / size.height();
            size = QSize(static_cast<int>(size.width() * scaleRatio), 300);
        }

        m_movie->setScaledSize(size);
        setMovie(m_movie);
        resize(size);
        m_movie->start();
    }

    void selectGifFile() {
        QString filePath = QFileDialog::getOpenFileName(this, "Select GIF", "", "GIF Files (*.gif)");
        if (!filePath.isEmpty()) {
            loadGif(filePath);
        } else if (!m_movie->isValid()) {
            qApp->quit();
        }
    }

protected:
    // --- Mouse Dragging Interaction ---
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

    // --- Right-Click Context Menu ---
    void contextMenuEvent(QContextMenuEvent *event) override {
        QMenu menu(this);
        QAction *changeAction = new QAction("Load New GIF...", this);
        connect(changeAction, &QAction::triggered, this, &DesktopDancer::selectGifFile);

        QAction *exitAction = new QAction("Exit App", this);
        connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

        menu.addAction(changeAction);
        menu.addSeparator();
        menu.addAction(exitAction);

        menu.exec(event->globalPos());
    }

    // --- Drag and Drop Logic ---
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        }
    }

    void dropEvent(QDropEvent *event) override {
        const QMimeData *mimeData = event->mimeData();
        if (mimeData->hasUrls()) {
            for (const QUrl &url : mimeData->urls()) {
                QString localFile = url.toLocalFile();
                if (QFileInfo(localFile).suffix().toLower() == "gif") {
                    loadGif(localFile);
                    event->acceptProposedAction();
                    break;
                }
            }
        }
    }

private:
    QMovie *m_movie;
    QPoint m_dragPosition;
};
#include "main.moc"
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString path = "";
    if (argc > 1) {
        path = QString::fromLocal8Bit(argv[1]);
    }

    DesktopDancer dancer(path);
    dancer.show();

    return app.exec();
}


