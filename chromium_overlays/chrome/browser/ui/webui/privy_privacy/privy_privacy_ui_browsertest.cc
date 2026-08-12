// Copyright 2026 The Privy Browser Authors

#include "base/test/run_until.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace {

class PrivyPrivacyUIBrowserTest : public InProcessBrowserTest {
 protected:
  content::WebContents* web_contents() {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }
};

IN_PROC_BROWSER_TEST_F(PrivyPrivacyUIBrowserTest,
                       FrequencyCapResultAppearsInPrivacyActivity) {
  ASSERT_TRUE(
      ui_test_utils::NavigateToURL(browser(), GURL("chrome://privacy")));

  EXPECT_EQ("Privacy Center",
            content::EvalJs(web_contents(), "document.title"));
  EXPECT_EQ("protected",
            content::EvalJs(web_contents(),
                           "document.getElementById('protection-status')"
                           ".textContent"));
  ASSERT_TRUE(base::test::RunUntil([&]() {
    return content::EvalJs(
               web_contents(),
               "document.getElementById('provider-status').textContent")
               .ExtractString() == "ready";
  }));

  content::EvalJsResult frequency_result = content::EvalJs(
      web_contents(), R"(
        new Promise(resolve => {
          const previousState = window.privyPrivacy.onState;
          window.privyPrivacy.onState = state => {
            previousState(state);
            if (state.activities.length === 1) {
              resolve(state.activities[0].decision);
            }
          };
          document.getElementById('run-frequency').click();
        })
      )");
  EXPECT_EQ("compute_privately", frequency_result.ExtractString());
  EXPECT_EQ("Frequency-cap result: allowed.",
            content::EvalJs(web_contents(),
                            "document.getElementById('frequency-result')"
                            ".textContent"));
  EXPECT_TRUE(content::EvalJs(
      web_contents(),
      "document.querySelector('.activity strong')"
      ".nextElementSibling.nextElementSibling.nextElementSibling"
      ".textContent.includes('browser')"));

  content::EvalJsResult clear_result = content::EvalJs(
      web_contents(), R"(
        new Promise(resolve => {
          const previousState = window.privyPrivacy.onState;
          window.privyPrivacy.onState = state => {
            previousState(state);
            if (state.activities.length === 0) {
              resolve(true);
            }
          };
          document.getElementById('clear-activity').click();
        })
      )");
  EXPECT_TRUE(clear_result.ExtractBool());
}

}  // namespace
