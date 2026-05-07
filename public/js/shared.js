(function () {
  'use strict';

  const SB = {};

  SB.getToken = function () {
    return sessionStorage.getItem('sb_token');
  };

  SB.getUser = function () {
    const raw = sessionStorage.getItem('sb_user');
    if (!raw) return null;
    try { return JSON.parse(raw); } catch (_) { return null; }
  };

  SB.setSession = function (token, user) {
    sessionStorage.setItem('sb_token', token);
    sessionStorage.setItem('sb_user', JSON.stringify(user));
  };

  SB.clearSession = function () {
    sessionStorage.removeItem('sb_token');
    sessionStorage.removeItem('sb_user');
  };

  SB.requireAuth = function () {
    const token = SB.getToken();
    const user = SB.getUser();
    if (!token || !user) {
      window.location.href = '/';
      return false;
    }
    return true;
  };

  SB.api = async function (path, opts) {
    opts = opts || {};
    const headers = Object.assign({}, opts.headers || {});
    if (!opts.skipAuth) {
      const token = SB.getToken();
      if (token) headers['Authorization'] = 'Bearer ' + token;
    }

    let body = opts.body;
    if (body && typeof body !== 'string') {
      headers['Content-Type'] = 'application/json';
      body = JSON.stringify(body);
    }

    let res;
    try {
      res = await fetch(path, {
        method: opts.method || 'GET',
        headers: headers,
        body: body
      });
    } catch (err) {
      const e = new Error('Cannot reach the server. Is it running?');
      e.status = 0;
      throw e;
    }

    if (res.status === 401) {
      SB.clearSession();
      if (window.location.pathname !== '/' &&
          window.location.pathname !== '/index.html') {
        window.location.href = '/';
      }
      const e = new Error('Session expired. Please sign in again.');
      e.status = 401;
      throw e;
    }

    let data = null;
    try { data = await res.json(); } catch (_) { }

    if (!res.ok) {
      const msg = (data && data.error) ? data.error : 'Request failed (' + res.status + ')';
      const e = new Error(msg);
      e.status = res.status;
      throw e;
    }
    return data;
  };

  SB.logout = async function () {
    try {
      await SB.api('/api/logout', { method: 'POST' });
    } catch (_) { }
    SB.clearSession();
    window.location.href = '/';
  };

  SB.toast = function (msg, opts) {
    opts = opts || {};
    let host = document.querySelector('.toast-host');
    if (!host) {
      host = document.createElement('div');
      host.className = 'toast-host';
      document.body.appendChild(host);
    }

    const t = document.createElement('div');
    t.className = 'toast' + (opts.error ? ' error' : '');

    const m = document.createElement('div');
    m.className = 'toast-msg';
    m.textContent = msg;

    const close = document.createElement('button');
    close.className = 'toast-close';
    close.setAttribute('aria-label', 'Dismiss');
    close.innerHTML = '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>';

    const dismiss = function () {
      t.classList.add('fade-out');
      setTimeout(function () { t.remove(); }, 250);
    };
    close.addEventListener('click', dismiss);

    t.appendChild(m);
    t.appendChild(close);
    host.appendChild(t);

    setTimeout(dismiss, opts.duration || 4000);
  };

  SB.debounce = function (fn, ms) {
    let h;
    return function () {
      const args = arguments;
      const ctx = this;
      clearTimeout(h);
      h = setTimeout(function () { fn.apply(ctx, args); }, ms);
    };
  };

  SB.initials = function (name) {
    if (!name) return '?';
    const parts = name.trim().split(/\s+/).filter(Boolean);
    if (parts.length === 0) return '?';
    if (parts.length === 1) return parts[0].charAt(0).toUpperCase();
    return (parts[0].charAt(0) + parts[parts.length - 1].charAt(0)).toUpperCase();
  };

  SB.formatPrice = function (amt) {
    const n = Number(amt) || 0;
    if (Number.isInteger(n)) {
      return 'Rs ' + n.toLocaleString('en-PK');
    }
    return 'Rs ' + n.toLocaleString('en-PK', {
      minimumFractionDigits: 2, maximumFractionDigits: 2
    });
  };

  SB.formatBalance = function (amt) {
    const n = Number(amt) || 0;
    if (Number.isInteger(n)) {
      return n.toLocaleString('en-PK');
    }
    return n.toLocaleString('en-PK', {
      minimumFractionDigits: 2, maximumFractionDigits: 2
    });
  };

  function escapeHtml(s) {
    return String(s)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#39;');
  }
  SB.escapeHtml = escapeHtml;
  SB.renderTopNav = function (opts) {
    opts = opts || {};
    const user = SB.getUser();
    if (!user) return;

    const mount = document.getElementById('topnav-mount');
    if (!mount) return;

    const role = user.role; 

    const links = [];
    if (role === 'ADMIN') {
      links.push({ key: 'browse', href: '/browse.html', label: 'Browse' });
      links.push({ key: 'admin',  href: '/admin.html',  label: 'Admin' });
    } else {
      links.push({ key: 'browse', href: '/browse.html', label: 'Browse' });
      if (role === 'FREELANCER') {
        links.push({ key: 'mygigs', href: '/my-gigs.html', label: 'My Gigs' });
      }
      links.push({ key: 'orders',       href: '/orders.html',       label: 'Orders' });
      links.push({ key: 'messages',     href: '/messages.html',     label: 'Messages' });
      links.push({ key: 'endorsements', href: '/endorsements.html', label: 'Endorsements' });
    }

    let html = '';
    html += '<nav class="nav">';

    html += '<a href="/browse.html" class="logo" aria-label="SkillBridge home">';
    html += '<span class="dot"></span>SkillBridge';
    html += '</a>';

    html += '<div class="nav-links">';
    for (const l of links) {
      const active = (l.key === opts.activeLink) ? ' class="active"' : '';
      html += '<a href="' + l.href + '"' + active + ' data-nav="' + l.key + '">';
      html += escapeHtml(l.label);
      html += '</a>';
    }
    html += '</div>';

    html += '<div class="nav-right">';

    if (role !== 'ADMIN' && typeof user.balance === 'number') {
      html += '<span class="balance-chip">Balance &middot; <b>Rs ' +
              SB.formatBalance(user.balance) + '</b></span>';
    }

    html += '<div class="avatar-wrap">';
    html += '<button id="avatar-btn" class="avatar" aria-label="Account menu" aria-haspopup="true" aria-expanded="false">';
    html += SB.initials(user.name);
    html += '</button>';

    html += '<div id="avatar-menu" class="avatar-menu" role="menu">';
    html += '<div class="avatar-menu-head">';
    html += '<div class="name">' + escapeHtml(user.name) + '</div>';
    html += '<div class="meta">' + escapeHtml(user.email) + ' &middot; ' + role + '</div>';
    html += '</div>';

    if (role !== 'ADMIN' && typeof user.balance === 'number') {
      html += '<div class="menu-balance">Balance &middot; <b>Rs ' +
              SB.formatBalance(user.balance) + '</b></div>';
      html += '<div class="avatar-menu-divider"></div>';
    }

    html += '<a href="/profile.html" class="avatar-menu-item" role="menuitem">';
    html += '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>';
    html += 'View profile</a>';
    html += '<div class="avatar-menu-divider"></div>';
    html += '<button id="logout-btn" class="avatar-menu-item danger" role="menuitem">';
    html += '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/><polyline points="16 17 21 12 16 7"/><line x1="21" y1="12" x2="9" y2="12"/></svg>';
    html += 'Sign out</button>';
    html += '</div>'; 
    html += '</div>'; 
    html += '</div>'; 
    html += '</nav>';

   mount.innerHTML = html;
    wireAvatarMenu();
     refreshBalance();
  };

  function refreshBalance() {
    SB.api('/api/me').then(function (data) {
      if (!data || !data.user) return;
      var old = SB.getUser();
      if (!old) return;
      old.balance = data.user.balance;
      sessionStorage.setItem('sb_user', JSON.stringify(old));
      var chips = document.querySelectorAll('.balance-chip');
      chips.forEach(function (chip) {
        chip.innerHTML = 'Balance &middot; <b>Rs ' + SB.formatBalance(data.user.balance) + '</b>';
      });
      var menuBal = document.querySelectorAll('.menu-balance');
      menuBal.forEach(function (el) {
        el.innerHTML = 'Balance &middot; <b>Rs ' + SB.formatBalance(data.user.balance) + '</b>';
      });
    }).catch(function () { });
  }

  function wireUnreadBadge() {
    var token = SB.getToken();
    if (!token) return;
    SB.api('/api/messages/unread').then(function (data) {
      if (!data || !data.unreadCount) return;
      var link = document.querySelector('[data-nav="messages"]');
      if (!link) return;
      var existing = link.querySelector('.nav-link-badge');
      if (existing) existing.remove();
      var badge = document.createElement('span');
      badge.className = 'nav-link-badge';
      badge.textContent = data.unreadCount > 99 ? '99+' : data.unreadCount;
      link.appendChild(badge);
    }).catch(function () {  });
  }

  function wireAvatarMenu() {
    const btn = document.getElementById('avatar-btn');
    const menu = document.getElementById('avatar-menu');
    const logoutBtn = document.getElementById('logout-btn');
    if (!btn || !menu) return;

    btn.addEventListener('click', function (e) {
      e.stopPropagation();
      const showing = menu.classList.toggle('show');
      btn.setAttribute('aria-expanded', showing ? 'true' : 'false');
    });

    document.addEventListener('click', function (e) {
      if (!menu.contains(e.target) && e.target !== btn) {
        menu.classList.remove('show');
        btn.setAttribute('aria-expanded', 'false');
      }
    });

    document.addEventListener('keydown', function (e) {
      if (e.key === 'Escape') {
        menu.classList.remove('show');
        btn.setAttribute('aria-expanded', 'false');
      }
    });

    if (logoutBtn) {
      logoutBtn.addEventListener('click', function () { SB.logout(); });
    }
  }

  SB.wireSearchBar = function (opts) {
    opts = opts || {};
    const input = document.getElementById('search-input');
    const goBtn = document.getElementById('search-go');
    const auto  = document.getElementById('search-autocomplete');
    if (!input) return;

    const submit = function () {
      const q = input.value.trim();
      if (!q) return;
      if (auto) auto.classList.remove('show');
      if (typeof opts.onSubmit === 'function') {
        opts.onSubmit(q);
      } else {
        window.location.href = '/search.html?q=' + encodeURIComponent(q);
      }
    };

    if (goBtn) goBtn.addEventListener('click', submit);

    input.addEventListener('keydown', function (e) {
      if (e.key === 'Enter') {
        e.preventDefault();
        submit();
      }
      if (e.key === 'Escape') {
        if (auto) auto.classList.remove('show');
        input.blur();
      }
    });

    if (typeof opts.onAutocomplete === 'function' && auto) {
      const debouncedFetch = SB.debounce(function (val) {
        opts.onAutocomplete(val, function (suggestions) {
          renderAutocomplete(auto, input, suggestions, opts);
        });
      }, 180);

      input.addEventListener('input', function () {
        const val = input.value.trim();
        if (val.length < 2) {
          auto.classList.remove('show');
          auto.innerHTML = '';
          return;
        }
        debouncedFetch(val);
      });

      document.addEventListener('click', function (e) {
        if (e.target !== input && !auto.contains(e.target)) {
          auto.classList.remove('show');
        }
      });

      input.addEventListener('focus', function () {
        if (auto.children.length > 0) auto.classList.add('show');
      });
    }
  };

  function renderAutocomplete(host, input, suggestions, opts) {
    if (!suggestions || suggestions.length === 0) {
      host.classList.remove('show');
      host.innerHTML = '';
      return;
    }
    let html = '';
    for (const s of suggestions) {
      html += '<button type="button" class="autocomplete-item" data-suggest="' +
              escapeHtml(s) + '">';
      html += '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>';
      html += escapeHtml(s);
      html += '</button>';
    }
    host.innerHTML = html;
    host.classList.add('show');

    host.querySelectorAll('.autocomplete-item').forEach(function (el) {
      el.addEventListener('click', function () {
        const q = el.dataset.suggest;
        input.value = q;
        host.classList.remove('show');
        if (typeof opts.onSubmit === 'function') {
          opts.onSubmit(q);
        } else {
          window.location.href = '/search.html?q=' + encodeURIComponent(q);
        }
      });
    });
  }

  window.SB = SB;
})();