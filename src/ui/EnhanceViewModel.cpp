#include "ui/EnhanceViewModel.h"

#include "app/EnhancementController.h"

#include <QDir>
#include <QFileInfo>

#include <chrono>
#include <utility>

namespace lumos::ui {

EnhanceViewModel::EnhanceViewModel(app::EnhancementController& controller, QObject* parent)
    : QObject(parent), controller_(controller) {
    poll_timer_.setInterval(40);
    connect(&poll_timer_, &QTimer::timeout, this, &EnhanceViewModel::pollPendingResult);
}

QString EnhanceViewModel::phase() const noexcept {
    return phase_;
}

QString EnhanceViewModel::statusText() const noexcept {
    return status_text_;
}

QString EnhanceViewModel::resultSummary() const noexcept {
    return result_summary_;
}

QString EnhanceViewModel::inputPath() const noexcept {
    return input_path_;
}

QString EnhanceViewModel::outputPath() const noexcept {
    return output_path_;
}

QUrl EnhanceViewModel::inputFileUrl() const {
    if (input_path_.isEmpty()) {
        return {};
    }
    return QUrl::fromLocalFile(input_path_);
}

QUrl EnhanceViewModel::resultFileUrl() const {
    if (output_path_.isEmpty()) {
        return {};
    }
    return QUrl::fromLocalFile(output_path_);
}

int EnhanceViewModel::scaleFactor() const noexcept {
    return scale_factor_;
}

bool EnhanceViewModel::denoiseEnabled() const noexcept {
    return denoise_enabled_;
}

bool EnhanceViewModel::busy() const noexcept {
    return busy_;
}

bool EnhanceViewModel::canEnhance() const noexcept {
    return !busy_ && !input_path_.isEmpty() && isPpmPath(input_path_);
}

bool EnhanceViewModel::hasResult() const noexcept {
    return has_result_;
}

bool EnhanceViewModel::hasError() const noexcept {
    return phase_ == "error";
}

void EnhanceViewModel::setInputPath(const QString& input_path) {
    QString normalized = input_path.trimmed();
    if (normalized.isEmpty()) {
        return;
    }

    normalized = localPathFromUrl(normalized);
    const QFileInfo info(normalized);
    if (!info.exists() || !info.isFile()) {
        if (!input_path_.isEmpty()) {
            input_path_.clear();
            emit inputPathChanged();
        }
        if (!output_path_.isEmpty()) {
            output_path_.clear();
            emit outputPathChanged();
        }
        if (has_result_) {
            has_result_ = false;
            emit hasResultChanged();
        }
        if (!result_summary_.isEmpty()) {
            result_summary_.clear();
            emit resultSummaryChanged();
        }
        emit canEnhanceChanged();
        setPhase("error");
        setStatus("Selected file was not found.");
        return;
    }

    normalized = QDir::toNativeSeparators(info.absoluteFilePath());
    const bool input_changed = (input_path_ != normalized);
    input_path_ = normalized;
    if (input_changed) {
        emit inputPathChanged();
    }

    controller_.trackInputSelected(input_path_.toStdString());

    if (has_result_) {
        has_result_ = false;
        emit hasResultChanged();
    }
    if (!result_summary_.isEmpty()) {
        result_summary_.clear();
        emit resultSummaryChanged();
    }

    refreshOutputPath();
    if (isPpmPath(input_path_)) {
        setPhase("ready");
        setStatus("Ready to enhance. Review settings and click Enhance.");
    } else {
        setPhase("error");
        setStatus("This prototype currently supports .ppm images.");
    }

    emit canEnhanceChanged();
}

void EnhanceViewModel::setScaleFactor(const int scale_factor) {
    if (!isSupportedScaleFactor(scale_factor) || scale_factor_ == scale_factor) {
        return;
    }

    scale_factor_ = scale_factor;
    emit scaleFactorChanged();
    refreshOutputPath();
    emit canEnhanceChanged();
}

void EnhanceViewModel::setDenoiseEnabled(const bool denoise_enabled) {
    if (denoise_enabled_ == denoise_enabled) {
        return;
    }

    denoise_enabled_ = denoise_enabled;
    emit denoiseEnabledChanged();
}

void EnhanceViewModel::startEnhancement() {
    if (busy_) {
        return;
    }

    if (!canEnhance()) {
        setPhase("error");
        setStatus("Select a valid .ppm image before running enhancement.");
        return;
    }

    if (output_path_.isEmpty()) {
        refreshOutputPath();
    }

    contracts::EnhancementRequest request;
    request.input_path = input_path_.toStdString();
    request.output_path = output_path_.toStdString();
    request.scale_factor = scale_factor_;
    request.denoise_enabled = denoise_enabled_;

    pending_result_.emplace(controller_.runEnhancementAsync(std::move(request)));

    busy_ = true;
    emit busyChanged();
    emit canEnhanceChanged();

    if (has_result_) {
        has_result_ = false;
        emit hasResultChanged();
    }
    if (!result_summary_.isEmpty()) {
        result_summary_.clear();
        emit resultSummaryChanged();
    }

    setPhase("running");
    setStatus("Enhancing image...");
    poll_timer_.start();
}

void EnhanceViewModel::resetSession() {
    if (busy_) {
        return;
    }

    const bool had_result = has_result_;
    const bool had_input = !input_path_.isEmpty();
    const bool had_output = !output_path_.isEmpty();
    const bool had_summary = !result_summary_.isEmpty();

    input_path_.clear();
    output_path_.clear();
    result_summary_.clear();
    has_result_ = false;

    setPhase("empty");
    setStatus("Drop a .ppm image to begin.");

    if (had_input) {
        emit inputPathChanged();
    }
    if (had_output) {
        emit outputPathChanged();
    }
    if (had_summary) {
        emit resultSummaryChanged();
    }
    if (had_result) {
        emit hasResultChanged();
    }
    emit canEnhanceChanged();
}

QString EnhanceViewModel::localPathFromUrl(const QString& raw_url) const {
    const QUrl url(raw_url);
    if (url.isLocalFile()) {
        return QDir::toNativeSeparators(url.toLocalFile());
    }
    return raw_url;
}

void EnhanceViewModel::pollPendingResult() {
    if (!pending_result_.has_value()) {
        poll_timer_.stop();
        return;
    }

    if (pending_result_->wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        return;
    }

    contracts::EnhancementResult result = pending_result_->get();
    pending_result_.reset();
    poll_timer_.stop();

    busy_ = false;
    emit busyChanged();
    emit canEnhanceChanged();

    if (result.ok) {
        const QString prior_output = output_path_;
        output_path_ = QString::fromStdString(result.output_path);
        if (prior_output != output_path_) {
            emit outputPathChanged();
        }

        has_result_ = true;
        emit hasResultChanged();

        result_summary_ = QString("%1x%2 -> %3x%4 in %5 ms")
                              .arg(result.metrics.input_width)
                              .arg(result.metrics.input_height)
                              .arg(result.metrics.output_width)
                              .arg(result.metrics.output_height)
                              .arg(static_cast<qulonglong>(result.metrics.duration_ms));
        emit resultSummaryChanged();

        setPhase("success");
        setStatus("Enhancement complete. Review result and output path.");
        return;
    }

    if (has_result_) {
        has_result_ = false;
        emit hasResultChanged();
    }
    if (!result_summary_.isEmpty()) {
        result_summary_.clear();
        emit resultSummaryChanged();
    }

    const QString stage = QString::fromStdString(result.error.stage);
    const QString message = QString::fromStdString(result.error.message);

    setPhase("error");
    if (message.isEmpty()) {
        setStatus(QString("Enhancement failed at stage: %1").arg(stage));
    } else {
        setStatus(QString("Enhancement failed at %1: %2").arg(stage, message));
    }
}

bool EnhanceViewModel::isSupportedScaleFactor(const int scale_factor) noexcept {
    return scale_factor == 2 || scale_factor == 4 || scale_factor == 8;
}

bool EnhanceViewModel::isPpmPath(const QString& local_path) {
    const QFileInfo info(local_path);
    return info.suffix().compare("ppm", Qt::CaseInsensitive) == 0;
}

void EnhanceViewModel::setPhase(const QString& next_phase) {
    if (phase_ == next_phase) {
        return;
    }

    const bool was_error = hasError();
    phase_ = next_phase;
    emit phaseChanged();

    if (was_error != hasError()) {
        emit hasErrorChanged();
    }
}

void EnhanceViewModel::setStatus(const QString& next_status) {
    if (status_text_ == next_status) {
        return;
    }

    status_text_ = next_status;
    emit statusTextChanged();
}

void EnhanceViewModel::refreshOutputPath() {
    QString next_output;
    if (!input_path_.isEmpty()) {
        const QFileInfo info(input_path_);
        const QString base_name = info.completeBaseName().isEmpty() ? QStringLiteral("output") : info.completeBaseName();
        next_output = QDir::toNativeSeparators(
            info.dir().absoluteFilePath(QString("%1_lumos_%2x.ppm").arg(base_name).arg(scale_factor_)));
    }

    if (output_path_ == next_output) {
        return;
    }

    output_path_ = next_output;
    emit outputPathChanged();
}

}  // namespace lumos::ui
