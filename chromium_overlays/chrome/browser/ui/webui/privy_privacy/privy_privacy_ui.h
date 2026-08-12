// Copyright 2026 The Privy Browser Authors

#ifndef CHROME_BROWSER_UI_WEBUI_PRIVY_PRIVACY_PRIVY_PRIVACY_UI_H_
#define CHROME_BROWSER_UI_WEBUI_PRIVY_PRIVACY_PRIVY_PRIVACY_UI_H_

#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/webui_config.h"
#include "content/public/common/url_constants.h"

class PrivyPrivacyUI;

class PrivyPrivacyUIConfig
    : public content::DefaultWebUIConfig<PrivyPrivacyUI> {
 public:
  PrivyPrivacyUIConfig()
      : DefaultWebUIConfig(content::kChromeUIScheme,
                           chrome::kChromeUIPrivyPrivacyHost) {}
};

class PrivyPrivacyUI : public content::WebUIController {
 public:
  explicit PrivyPrivacyUI(content::WebUI* web_ui);

  PrivyPrivacyUI(const PrivyPrivacyUI&) = delete;
  PrivyPrivacyUI& operator=(const PrivyPrivacyUI&) = delete;

  ~PrivyPrivacyUI() override;
};

#endif  // CHROME_BROWSER_UI_WEBUI_PRIVY_PRIVACY_PRIVY_PRIVACY_UI_H_
