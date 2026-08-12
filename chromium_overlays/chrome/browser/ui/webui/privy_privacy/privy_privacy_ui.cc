// Copyright 2026 The Privy Browser Authors

#include "chrome/browser/ui/webui/privy_privacy/privy_privacy_ui.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "base/check.h"
#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "base/values.h"
#include "chrome/browser/privy/privacy/privy_privacy_activity.h"
#include "chrome/browser/privy/privacy/privy_privacy_service.h"
#include "chrome/browser/privy/privacy/privy_privacy_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/grit/privy_privacy_resources.h"
#include "chrome/grit/privy_privacy_resources_map.h"
#include "components/privy_privacy/compute/private_compute_types.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "ui/webui/webui_util.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace {

base::DictValue ActivityToDict(const privy::PrivacyActivity& activity) {
  return base::DictValue()
      .Set("timestamp", activity.timestamp.ToJsTime())
      .Set("requester", activity.requester.Serialize())
      .Set("requester_type",
           privy::RequesterTypeToString(activity.requester_type))
      .Set("surface", privy::PrivacySurfaceToString(activity.surface))
      .Set("decision", privy::PrivacyActionToString(activity.decision))
      .Set("private_input_type",
           privy::PrivacyActivityPrivateInputTypeToString(
               activity.private_input_type))
      .Set("private_input_count",
           static_cast<int>(activity.private_input_count))
      .Set("disclosed_type",
           privy::PrivacyActivityDisclosureTypeToString(
               activity.disclosed_type))
      .Set("disclosed_field_count",
           static_cast<int>(activity.disclosed_field_count))
      .Set("private_compute_attempted", activity.private_compute_attempted)
      .Set("private_compute_succeeded", activity.private_compute_succeeded)
      .Set("reason", activity.reason);
}

class PrivyPrivacyHandler final : public content::WebUIMessageHandler,
                                  public privy::PrivyPrivacyService::Observer {
 public:
  explicit PrivyPrivacyHandler(Profile* profile)
      : service_(privy::PrivyPrivacyServiceFactory::GetForProfile(profile)) {
    CHECK(service_);
    service_observation_.Observe(service_);
  }

  PrivyPrivacyHandler(const PrivyPrivacyHandler&) = delete;
  PrivyPrivacyHandler& operator=(const PrivyPrivacyHandler&) = delete;

  ~PrivyPrivacyHandler() override = default;

  void RegisterMessages() override {
    web_ui()->RegisterMessageCallback(
        "getPrivacyState",
        base::BindRepeating(&PrivyPrivacyHandler::HandleGetPrivacyState,
                            base::Unretained(this)));
    web_ui()->RegisterMessageCallback(
        "clearActivity",
        base::BindRepeating(&PrivyPrivacyHandler::HandleClearActivity,
                            base::Unretained(this)));
    web_ui()->RegisterMessageCallback(
        "runFrequencyCap",
        base::BindRepeating(&PrivyPrivacyHandler::HandleRunFrequencyCap,
                            base::Unretained(this)));
  }

  void OnJavascriptAllowed() override { SendState(); }

  void OnJavascriptDisallowed() override {}

  void OnPrivacyActivityAdded(const privy::PrivacyActivity&) override {
    SendState();
  }

  void OnPrivacyActivityCleared() override { SendState(); }

 private:
  void HandleGetPrivacyState(const base::ListValue&) {
    AllowJavascript();
    SendState();
  }

  void HandleClearActivity(const base::ListValue&) {
    AllowJavascript();
    service_->ClearActivity();
  }

  void HandleRunFrequencyCap(const base::ListValue&) {
    AllowJavascript();

    privy::PrivacyRequest privacy_request;
    privacy_request.requester =
        url::Origin::Create(GURL("chrome://privacy"));
    privacy_request.requester_type = privy::RequesterType::kBrowser;
    privacy_request.surface = privy::PrivacySurface::kFrequencyCap;
    privacy_request.contains_behavioral_data = true;

    privy::PrivateComputeRequest compute_request;
    compute_request.requester = privacy_request.requester;
    compute_request.capability =
        privy::PrivateComputeCapability::kFrequencyCap;
    compute_request.input = privy::FrequencyCapInput{
        .campaign_commitment = 0x5052495659,
        .prior_impressions = 2,
        .maximum_impressions = 3,
    };

    service_->EvaluateAndMaybeCompute(
        std::move(privacy_request), std::move(compute_request),
        base::BindOnce(&PrivyPrivacyHandler::OnFrequencyCapComplete,
                       weak_factory_.GetWeakPtr()));
  }

  void OnFrequencyCapComplete(
      privy::PrivacyDecision decision,
      std::optional<privy::PrivateComputeResult> result) {
    bool allowed = false;
    if (decision.action == privy::PrivacyAction::kComputePrivately &&
        result && result->success &&
        std::holds_alternative<bool>(result->value)) {
      allowed = std::get<bool>(result->value);
    }
    CallJavascriptFunction("privyPrivacy.onFrequencyCapResult",
                           base::Value(allowed));
    SendState();
  }

  base::DictValue BuildState() const {
    base::DictValue counts;
    counts.Set("allow",
               static_cast<int>(service_->GetActivityCount(
                   privy::PrivacyAction::kAllow)));
    counts.Set("blocked",
               static_cast<int>(service_->GetActivityCount(
                   privy::PrivacyAction::kBlock)));
    counts.Set("sanitized",
               static_cast<int>(service_->GetActivityCount(
                   privy::PrivacyAction::kSanitize)));
    counts.Set("prompt",
               static_cast<int>(service_->GetActivityCount(
                   privy::PrivacyAction::kPrompt)));
    counts.Set("compute_privately",
               static_cast<int>(service_->GetActivityCount(
                   privy::PrivacyAction::kComputePrivately)));

    base::ListValue activities;
    for (const privy::PrivacyActivity& activity :
         service_->GetRecentActivity()) {
      activities.Append(ActivityToDict(activity));
    }

    return base::DictValue()
        .Set("protection_status", "protected")
        .Set("provider_status",
             service_->SupportsPrivateCompute(
                 privy::PrivateComputeCapability::kFrequencyCap)
                 ? "ready"
                 : "unavailable")
        .Set("off_the_record", service_->off_the_record())
        .Set("counts", std::move(counts))
        .Set("activities", std::move(activities));
  }

  void SendState() {
    if (!IsJavascriptAllowed()) {
      return;
    }
    CallJavascriptFunction("privyPrivacy.onState", BuildState());
  }

  raw_ptr<privy::PrivyPrivacyService> service_;
  base::ScopedObservation<privy::PrivyPrivacyService,
                           privy::PrivyPrivacyService::Observer>
      service_observation_{this};
  base::WeakPtrFactory<PrivyPrivacyHandler> weak_factory_{this};
};

}  // namespace

PrivyPrivacyUI::PrivyPrivacyUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  Profile* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      profile, chrome::kChromeUIPrivyPrivacyHost);
  webui::SetupWebUIDataSource(source, kPrivyPrivacyResources,
                              IDR_PRIVY_PRIVACY_PRIVACY_HTML);
  web_ui->AddMessageHandler(std::make_unique<PrivyPrivacyHandler>(profile));
}

PrivyPrivacyUI::~PrivyPrivacyUI() = default;
