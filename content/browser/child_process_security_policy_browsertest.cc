// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/basictypes.h"
#include "base/file_path.h"
#include "base/process_util.h"
#include "content/browser/child_process_security_policy_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/result_codes.h"
#include "content/shell/shell.h"
#include "content/test/content_browser_test.h"
#include "content/test/content_browser_test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

class ChildProcessSecurityPolicyInProcessBrowserTest
    : public content::ContentBrowserTest {
 public:
  virtual void SetUp() {
    EXPECT_EQ(
      ChildProcessSecurityPolicyImpl::GetInstance()->security_state_.size(),
          0U);
    ContentBrowserTest::SetUp();
  }

  virtual void TearDown() {
    EXPECT_EQ(
      ChildProcessSecurityPolicyImpl::GetInstance()->security_state_.size(),
          0U);
    ContentBrowserTest::TearDown();
  }
};

#if !defined(NDEBUG) && defined(OS_MACOSX)
IN_PROC_BROWSER_TEST_F(ChildProcessSecurityPolicyInProcessBrowserTest, DISABLED_NoLeak) {
#else
IN_PROC_BROWSER_TEST_F(ChildProcessSecurityPolicyInProcessBrowserTest, NoLeak) {
#endif
  GURL url = content::GetTestUrl("", "simple_page.html");

  content::NavigateToURL(shell(), url);
  EXPECT_EQ(
      ChildProcessSecurityPolicyImpl::GetInstance()->security_state_.size(),
          1U);

  content::WebContents* web_contents = shell()->web_contents();
  base::KillProcess(web_contents->GetRenderProcessHost()->GetHandle(),
                    content::RESULT_CODE_KILLED, true);

  web_contents->GetController().Reload(true);
  EXPECT_EQ(
      1U,
      ChildProcessSecurityPolicyImpl::GetInstance()->security_state_.size());
}
