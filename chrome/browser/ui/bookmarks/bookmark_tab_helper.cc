// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/bookmarks/bookmark_tab_helper.h"

#include "chrome/browser/bookmarks/bookmark_model.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/bookmarks/bookmark_tab_helper_delegate.h"
#include "chrome/browser/ui/tab_contents/tab_contents_wrapper.h"
#include "content/browser/tab_contents/navigation_controller.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/browser/webui/web_ui.h"
#include "content/common/notification_service.h"

BookmarkTabHelper::BookmarkTabHelper(TabContentsWrapper* tab_contents)
    : TabContentsObserver(tab_contents->tab_contents()),
      is_starred_(false),
      tab_contents_wrapper_(tab_contents),
      delegate_(NULL) {
  // Register for notifications about URL starredness changing on any profile.
  registrar_.Add(this, NotificationType::URLS_STARRED,
                 NotificationService::AllSources());
  registrar_.Add(this, NotificationType::BOOKMARK_MODEL_LOADED,
                 NotificationService::AllSources());
}

BookmarkTabHelper::~BookmarkTabHelper() {
  // We don't want any notifications while we're running our destructor.
  registrar_.RemoveAll();
}

bool BookmarkTabHelper::ShouldShowBookmarkBar() {
  if (tab_contents()->showing_interstitial_page())
    return false;

  // See TabContents::GetWebUIForCurrentState() comment for more info. This case
  // is very similar, but for non-first loads, we want to use the committed
  // entry. This is so the bookmarks bar disappears at the same time the page
  // does.
  if (tab_contents()->controller().GetLastCommittedEntry()) {
    // Not the first load, always use the committed Web UI.
    return tab_contents()->committed_web_ui() &&
        tab_contents()->committed_web_ui()->force_bookmark_bar_visible();
  }

  // When it's the first load, we know either the pending one or the committed
  // one will have the Web UI in it (see GetWebUIForCurrentState), and only one
  // of them will be valid, so we can just check both.
  return tab_contents()->web_ui() &&
      tab_contents()->web_ui()->force_bookmark_bar_visible();
}

void BookmarkTabHelper::DidNavigateMainFramePostCommit(
    const content::LoadCommittedDetails& /*details*/,
    const ViewHostMsg_FrameNavigate_Params& /*params*/) {
  UpdateStarredStateForCurrentURL();
}

void BookmarkTabHelper::Observe(NotificationType type,
                                const NotificationSource& source,
                                const NotificationDetails& details) {
  switch (type.value) {
    case NotificationType::BOOKMARK_MODEL_LOADED:
      // BookmarkModel finished loading, fall through to update starred state.
    case NotificationType::URLS_STARRED: {
      // Somewhere, a URL has been starred.
      // Ignore notifications for profiles other than our current one.
      Profile* source_profile = Source<Profile>(source).ptr();
      if (!source_profile ||
          !source_profile->IsSameProfile(tab_contents_wrapper_->profile()))
        return;

      UpdateStarredStateForCurrentURL();
      break;
    }

    default:
      NOTREACHED();
  }
}

void BookmarkTabHelper::UpdateStarredStateForCurrentURL() {
  BookmarkModel* model = tab_contents()->profile()->GetBookmarkModel();
  const bool old_state = is_starred_;
  is_starred_ = (model && model->IsBookmarked(tab_contents()->GetURL()));

  if (is_starred_ != old_state && delegate())
    delegate()->URLStarredChanged(tab_contents_wrapper_, is_starred_);
}
