// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_SYNC_SENDER_H__
#define IPC_IPC_SYNC_SENDER_H__
#pragma once

#include <string>
#include <deque>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event_watcher.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_sync_message.h"

namespace base {
class WaitableEvent;
};

namespace IPC {

class SyncMessage;
class MessageReplyDeserializer;

// This is similar to ChannelProxy, with the added feature of supporting sending
// synchronous messages.
//
// Overview of how the sync channel works
// --------------------------------------
// When the sending thread sends a synchronous message, we create a bunch
// of tracking info (created in SendWithTimeout, stored in the PendingSyncMsg
// structure) associated with the message that we identify by the unique
// "MessageId" on the SyncMessage. Among the things we save is the
// "Deserializer" which is provided by the sync message. This object is in
// charge of reading the parameters from the reply message and putting them in
// the output variables provided by its caller.
//
// The info gets stashed in a queue since we could have a nested stack of sync
// messages (each side could send sync messages in response to sync messages,
// so it works like calling a function). The message is sent to the I/O thread
// for dispatch and the original thread blocks waiting for the reply.
//
// SyncContext maintains the queue in a threadsafe way and listens for replies
// on the I/O thread. When a reply comes in that matches one of the messages
// it's looking for (using the unique message ID), it will execute the
// deserializer stashed from before, and unblock the original thread.
//
//
// Significant complexity results from the fact that messages are still coming
// in while the original thread is blocked. Normal async messages are queued
// and dispatched after the blocking call is complete. Sync messages must
// be dispatched in a reentrant manner to avoid deadlock.
//
//
// Note that care must be taken that the lifetime of the ipc_thread argument
// is more than this object.  If the message loop goes away while this object
// is running and it's used to send a message, then it will use the invalid
// message loop pointer to proxy it to the ipc thread.
class SyncChannel : public ChannelProxy,
                    public base::WaitableEventWatcher::Delegate {
 public:
  SyncChannel(const IPC::ChannelHandle& channel_handle,
              Channel::Mode mode,
              Channel::Listener* listener,
              base::MessageLoopProxy* ipc_message_loop,
              bool create_pipe_now,
              base::WaitableEvent* shutdown_event);
  virtual ~SyncChannel();

  virtual bool Send(Message* message);
  virtual bool SendWithTimeout(Message* message, int timeout_ms);

  // Whether we allow sending messages with no time-out.
  void set_sync_messages_with_no_timeout_allowed(bool value) {
    sync_messages_with_no_timeout_allowed_ = value;
  }

  // Sets this channel to only dispatch its incoming unblocking messages when it
  // is itself blocked on sending a sync message, not when other channels are.
  //
  // Normally, any unblocking message coming from any channel can be dispatched
  // when any (possibly other) channel is blocked on sending a message. This is
  // needed in some cases to unblock certain loops (e.g. necessary when some
  // processes share a window hierarchy), but may cause re-entrancy issues in
  // some cases where such loops are not possible. This flags allows the tagging
  // of some particular channels to not re-enter in such cases.
  void SetRestrictDispatchToSameChannel(bool value);

 protected:
  class ReceivedSyncMsgQueue;
  friend class ReceivedSyncMsgQueue;

  // SyncContext holds the per object data for SyncChannel, so that SyncChannel
  // can be deleted while it's being used in a different thread.  See
  // ChannelProxy::Context for more information.
  class SyncContext : public Context,
                      public base::WaitableEventWatcher::Delegate {
   public:
    SyncContext(Channel::Listener* listener,
                base::MessageLoopProxy* ipc_thread,
                base::WaitableEvent* shutdown_event);

    // Adds information about an outgoing sync message to the context so that
    // we know how to deserialize the reply.
    void Push(SyncMessage* sync_msg);

    // Cleanly remove the top deserializer (and throw it away).  Returns the
    // result of the Send call for that message.
    bool Pop();

    // Returns an event that's set when the send is complete, timed out or the
    // process shut down.
    base::WaitableEvent* GetSendDoneEvent();

    // Returns an event that's set when an incoming message that's not the reply
    // needs to get dispatched (by calling SyncContext::DispatchMessages).
    base::WaitableEvent* GetDispatchEvent();

    void DispatchMessages();

    // Checks if the given message is blocking the listener thread because of a
    // synchronous send.  If it is, the thread is unblocked and true is
    // returned. Otherwise the function returns false.
    bool TryToUnblockListener(const Message* msg);

    // Called on the IPC thread when a sync send that runs a nested message loop
    // times out.
    void OnSendTimeout(int message_id);

    base::WaitableEvent* shutdown_event() { return shutdown_event_; }

    ReceivedSyncMsgQueue* received_sync_msgs() {
      return received_sync_msgs_;
    }

    void set_restrict_dispatch(bool value) { restrict_dispatch_ = value; }
    bool restrict_dispatch() const { return restrict_dispatch_; }

   private:
    ~SyncContext();
    // ChannelProxy methods that we override.

    // Called on the listener thread.
    virtual void Clear();

    // Called on the IPC thread.
    virtual bool OnMessageReceived(const Message& msg);
    virtual void OnChannelError();
    virtual void OnChannelOpened();
    virtual void OnChannelClosed();

    // Cancels all pending Send calls.
    void CancelPendingSends();

    // WaitableEventWatcher::Delegate implementation.
    virtual void OnWaitableEventSignaled(base::WaitableEvent* arg);

    typedef std::deque<PendingSyncMsg> PendingSyncMessageQueue;
    PendingSyncMessageQueue deserializers_;
    base::Lock deserializers_lock_;

    scoped_refptr<ReceivedSyncMsgQueue> received_sync_msgs_;

    base::WaitableEvent* shutdown_event_;
    base::WaitableEventWatcher shutdown_watcher_;
    bool restrict_dispatch_;
  };

 private:
  // WaitableEventWatcher::Delegate implementation.
  virtual void OnWaitableEventSignaled(base::WaitableEvent* arg);

  SyncContext* sync_context() {
    return reinterpret_cast<SyncContext*>(context());
  }

  // Both these functions wait for a reply, timeout or process shutdown.  The
  // latter one also runs a nested message loop in the meantime.
  static void WaitForReply(
      SyncContext* context, base::WaitableEvent* pump_messages_event);

  // Runs a nested message loop until a reply arrives, times out, or the process
  // shuts down.
  static void WaitForReplyWithNestedMessageLoop(SyncContext* context);

  bool sync_messages_with_no_timeout_allowed_;

  // Used to signal events between the IPC and listener threads.
  base::WaitableEventWatcher dispatch_watcher_;

  DISALLOW_COPY_AND_ASSIGN(SyncChannel);
};

}  // namespace IPC

#endif  // IPC_IPC_SYNC_SENDER_H__
