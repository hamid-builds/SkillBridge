(function () {
  'use strict';

  if (!SB.requireAuth()) return;
  var user = SB.getUser();
  SB.renderTopNav({ activeLink: 'messages' });

  var state = {
    activePartnerID: null,
    activePartnerName: '',
    conversations: []  
  };

  var params = new URLSearchParams(window.location.search);
  var withID = params.get('with');
  if (withID) {
    withID = parseInt(withID, 10);
    if (!isFinite(withID) || withID <= 0) withID = null;
  }

  loadInbox().then(function () {
    if (withID) {
      openConversation(withID);
    }
  });

  async function loadInbox() {
    showInboxSkeleton();
    try {
      var data = await SB.api('/api/messages/inbox');
      state.conversations = data.conversations || [];
      renderInbox();
    } catch (err) {
      SB.toast(err.message, { error: true });
      state.conversations = [];
      renderInbox();
    }
  }

  function renderInbox() {
    var list = document.getElementById('inboxList');

    if (state.conversations.length === 0) {
      list.innerHTML = '<div class="inbox-empty">No conversations yet. Message someone from their gig or profile page.</div>';
      return;
    }

    var html = '';
    for (var i = 0; i < state.conversations.length; i++) {
      var c = state.conversations[i];
      var active = (c.otherPartyID === state.activePartnerID) ? ' active' : '';
      var preview = c.isLastMine ? 'You: ' + c.lastMessage : c.lastMessage;

      html += '<button class="inbox-item' + active + '" data-partnerid="' + c.otherPartyID + '" data-partnername="' + SB.escapeHtml(c.otherPartyName) + '">';
      html += '<div class="mini-avatar">' + SB.escapeHtml(SB.initials(c.otherPartyName)) + '</div>';
      html += '<div class="inbox-info">';
      html += '<div class="inbox-name">' + SB.escapeHtml(c.otherPartyName) + '</div>';
      html += '<div class="inbox-preview">' + SB.escapeHtml(preview) + '</div>';
      html += '</div>';
      html += '<div class="inbox-right">';
      html += '<span class="inbox-time">' + formatTime(c.lastTimestamp) + '</span>';
      if (c.unreadCount > 0) {
        html += '<span class="inbox-badge">' + c.unreadCount + '</span>';
      }
      html += '</div>';
      html += '</button>';
    }
    list.innerHTML = html;

    list.querySelectorAll('.inbox-item').forEach(function (item) {
      item.addEventListener('click', function () {
        var pid = parseInt(item.dataset.partnerid, 10);
        openConversation(pid, item.dataset.partnername);
      });
    });
  }

  async function openConversation(partnerID, partnerName) {
    state.activePartnerID = partnerID;
    state.activePartnerName = partnerName || '';

    document.querySelectorAll('.inbox-item').forEach(function (el) {
      el.classList.toggle('active', parseInt(el.dataset.partnerid, 10) === partnerID);
      if (parseInt(el.dataset.partnerid, 10) === partnerID && !partnerName) {
        state.activePartnerName = el.dataset.partnername;
      }
    });

    document.getElementById('msgLayout').classList.add('conv-open');

    var msgContainer = document.getElementById('convMessages');
    var emptyEl = document.getElementById('convEmpty');
    var composeBar = document.getElementById('composeBar');
    var nameEl = document.getElementById('convName');
    var profileLink = document.getElementById('convProfileLink');

    nameEl.textContent = state.activePartnerName || 'Loading...';
    profileLink.hidden = true;
    emptyEl.style.display = 'none';
    composeBar.hidden = true;
    msgContainer.innerHTML = '<div style="padding:40px;text-align:center;color:var(--text-faint)">Loading messages...</div>';

    try {
      var data = await SB.api('/api/messages/conversation/' + partnerID);
      var messages = data.messages || [];
      var other = data.otherParty || {};

      state.activePartnerName = other.name || state.activePartnerName;
      nameEl.textContent = state.activePartnerName;
      profileLink.href = '/profile.html?id=' + other.userID;
      profileLink.hidden = false;
      composeBar.hidden = false;

      renderMessages(messages);

      loadInbox();
    } catch (err) {
      if (err.status === 404) {
        nameEl.textContent = state.activePartnerName || 'User #' + partnerID;
        profileLink.href = '/profile.html?id=' + partnerID;
        profileLink.hidden = false;
        composeBar.hidden = false;
        msgContainer.innerHTML = '';
        emptyEl.style.display = 'grid';
        document.getElementById('convEmptyText').textContent = 'Start a conversation!';

        try {
          var udata = await SB.api('/api/users/' + partnerID);
          if (udata && udata.user) {
            state.activePartnerName = udata.user.name;
            nameEl.textContent = state.activePartnerName;
          }
        } catch (_) {}
        return;
      }
      msgContainer.innerHTML = '<div style="padding:40px;text-align:center;color:var(--error)">' + SB.escapeHtml(err.message) + '</div>';
    }
  }

  function renderMessages(messages) {
    var container = document.getElementById('convMessages');
    var emptyEl = document.getElementById('convEmpty');

    if (messages.length === 0) {
      container.innerHTML = '';
      emptyEl.style.display = 'grid';
      document.getElementById('convEmptyText').textContent = 'Start a conversation!';
      return;
    }

    emptyEl.style.display = 'none';
    var html = '';
    for (var i = 0; i < messages.length; i++) {
      var m = messages[i];
      var mine = m.senderID === user.userID;
      html += '<div class="msg-bubble ' + (mine ? 'mine' : 'theirs') + '">';
      html += '<div>' + SB.escapeHtml(m.text) + '</div>';
      html += '<div class="msg-time">' + formatTime(m.timestamp) + '</div>';
      html += '</div>';
    }
    container.innerHTML = html;

    requestAnimationFrame(function () {
      container.scrollTop = container.scrollHeight;
    });
  }

  var composeInput = document.getElementById('composeInput');
  var composeSend = document.getElementById('composeSend');

  composeSend.addEventListener('click', sendMessage);
  composeInput.addEventListener('keydown', function (e) {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      sendMessage();
    }
  });

  composeInput.addEventListener('input', function () {
    this.style.height = 'auto';
    this.style.height = Math.min(this.scrollHeight, 120) + 'px';
  });

  async function sendMessage() {
    var text = composeInput.value.trim();
    if (!text || !state.activePartnerID) return;

    composeSend.disabled = true;
    try {
      var data = await SB.api('/api/messages', {
        method: 'POST',
        body: { receiverID: state.activePartnerID, text: text }
      });

      composeInput.value = '';
      composeInput.style.height = 'auto';

      var container = document.getElementById('convMessages');
      var emptyEl = document.getElementById('convEmpty');
      emptyEl.style.display = 'none';

      var m = data.message;
      var bubble = document.createElement('div');
      bubble.className = 'msg-bubble mine';
      bubble.innerHTML = '<div>' + SB.escapeHtml(m.text) + '</div>' +
                         '<div class="msg-time">' + formatTime(m.timestamp) + '</div>';
      container.appendChild(bubble);
      container.scrollTop = container.scrollHeight;

      loadInbox();
    } catch (err) {
      SB.toast(err.message, { error: true });
    } finally {
      composeSend.disabled = false;
      composeInput.focus();
    }
  }

  document.getElementById('backBtn').addEventListener('click', function () {
    document.getElementById('msgLayout').classList.remove('conv-open');
    state.activePartnerID = null;
    loadInbox();
  });

  function formatTime(ts) {
    if (!ts) return '';
    var d = new Date(ts.replace(' ', 'T'));
    if (isNaN(d.getTime())) return ts;
    var now = new Date();
    var diff = now - d;
    if (diff < 86400000 && d.getDate() === now.getDate()) {
      return d.toLocaleTimeString('en-US', { hour: 'numeric', minute: '2-digit' });
    }
    if (diff < 604800000) {
      return d.toLocaleDateString('en-US', { weekday: 'short' });
    }
    return d.toLocaleDateString('en-US', { month: 'short', day: 'numeric' });
  }

  function showInboxSkeleton() {
    var list = document.getElementById('inboxList');
    var html = '';
    for (var i = 0; i < 5; i++) {
      html += '<div class="skel-inbox-item">';
      html += '<div class="skeleton si-av"></div>';
      html += '<div class="si-info">';
      html += '<div class="skeleton si-n"></div>';
      html += '<div class="skeleton si-p"></div>';
      html += '</div></div>';
    }
    list.innerHTML = html;
  }

})();