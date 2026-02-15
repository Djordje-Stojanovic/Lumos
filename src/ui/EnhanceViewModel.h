#pragma once

#include "contracts/EnhancementTypes.h"

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>

#include <future>
#include <optional>

namespace lumos::app {
class EnhancementController;
}

namespace lumos::ui {

class EnhanceViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString phase READ phase NOTIFY phaseChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString resultSummary READ resultSummary NOTIFY resultSummaryChanged)
    Q_PROPERTY(QString inputPath READ inputPath NOTIFY inputPathChanged)
    Q_PROPERTY(QString outputPath READ outputPath NOTIFY outputPathChanged)
    Q_PROPERTY(QUrl inputFileUrl READ inputFileUrl NOTIFY inputPathChanged)
    Q_PROPERTY(QUrl resultFileUrl READ resultFileUrl NOTIFY outputPathChanged)
    Q_PROPERTY(int scaleFactor READ scaleFactor NOTIFY scaleFactorChanged)
    Q_PROPERTY(bool denoiseEnabled READ denoiseEnabled NOTIFY denoiseEnabledChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool canEnhance READ canEnhance NOTIFY canEnhanceChanged)
    Q_PROPERTY(bool hasResult READ hasResult NOTIFY hasResultChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY hasErrorChanged)

  public:
    explicit EnhanceViewModel(app::EnhancementController& controller, QObject* parent = nullptr);
    ~EnhanceViewModel() override = default;

    [[nodiscard]] QString phase() const noexcept;
    [[nodiscard]] QString statusText() const noexcept;
    [[nodiscard]] QString resultSummary() const noexcept;
    [[nodiscard]] QString inputPath() const noexcept;
    [[nodiscard]] QString outputPath() const noexcept;
    [[nodiscard]] QUrl inputFileUrl() const;
    [[nodiscard]] QUrl resultFileUrl() const;
    [[nodiscard]] int scaleFactor() const noexcept;
    [[nodiscard]] bool denoiseEnabled() const noexcept;
    [[nodiscard]] bool busy() const noexcept;
    [[nodiscard]] bool canEnhance() const noexcept;
    [[nodiscard]] bool hasResult() const noexcept;
    [[nodiscard]] bool hasError() const noexcept;

    Q_INVOKABLE void setInputPath(const QString& input_path);
    Q_INVOKABLE void setScaleFactor(int scale_factor);
    Q_INVOKABLE void setDenoiseEnabled(bool denoise_enabled);
    Q_INVOKABLE void startEnhancement();
    Q_INVOKABLE void resetSession();
    Q_INVOKABLE QString localPathFromUrl(const QString& raw_url) const;

  signals:
    void phaseChanged();
    void statusTextChanged();
    void resultSummaryChanged();
    void inputPathChanged();
    void outputPathChanged();
    void scaleFactorChanged();
    void denoiseEnabledChanged();
    void busyChanged();
    void canEnhanceChanged();
    void hasResultChanged();
    void hasErrorChanged();

  private slots:
    void pollPendingResult();

  private:
    static bool isSupportedScaleFactor(int scale_factor) noexcept;
    static bool isPpmPath(const QString& local_path);

    void setPhase(const QString& next_phase);
    void setStatus(const QString& next_status);
    void refreshOutputPath();

    app::EnhancementController& controller_;
    QTimer poll_timer_;
    std::optional<std::future<contracts::EnhancementResult>> pending_result_;

    QString phase_ {"empty"};
    QString status_text_ {"Drop a .ppm image to begin."};
    QString result_summary_;
    QString input_path_;
    QString output_path_;
    int scale_factor_ {4};
    bool denoise_enabled_ {true};
    bool busy_ {false};
    bool has_result_ {false};
};

}  // namespace lumos::ui
