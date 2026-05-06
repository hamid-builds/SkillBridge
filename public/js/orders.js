(function () {
  'use strict';

  if (!SB.requireAuth()) return;
  const user = SB.getUser();
  SB.renderTopNav({ activeLink: 'orders' });

  var state = {
    tab: '', 
    sort: 'newest',
    orders: []
  };

  var tabs = [];
  if (user.role === 'CLIENT') {
    tabs = [{ key: 'buyer', label: 'My Orders' }];
  } else if (user.role === 'FREELANCER') {
    tabs = [{ key: 'seller', label: 'Incoming Orders' }];
  } else if (user.role === 'ADMIN') {
    tabs = [
      { key: 'buyer', label: 'As Buyer' },
      { key: 'seller', label: 'As Seller' },
      { key: 'all', label: 'All Orders' }
    ];
  }
  state.tab = tabs[0].key;

  renderTabs();
  fetchOrders();

  function renderTabs() {
    var mount = document.getElementById('orderTabs');
    if (tabs.length <= 1) {
      mount.style.display = 'none';
      return;
    }
    var html = '';
    for (var i = 0; i < tabs.length; i++) {
      var t = tabs[i];
      var cls = t.key === state.tab ? ' on' : '';
      html += '<button data-tab="' + t.key + '" class="' + cls + '">' +
              SB.escapeHtml(t.label) + '</button>';
    }
    mount.innerHTML = html;
    mount.querySelectorAll('button').forEach(function (btn) {
      btn.addEventListener('click', function () {
        state.tab = btn.dataset.tab;
        renderTabs();
        fetchOrders();
      });
    });
  }

  document.getElementById('sortToggle').addEventListener('click', function (e) {
    var btn = e.target.closest('button[data-sort]');
    if (!btn) return;
    state.sort = btn.dataset.sort;
    document.querySelectorAll('#sortToggle button').forEach(function (b) {
      b.classList.toggle('on', b.dataset.sort === state.sort);
    });
    renderOrders();
  });

  async function fetchOrders() {
    showSkeleton();
    try {
      var data = await SB.api('/api/orders?role=' + state.tab);
      state.orders = data.orders || [];
      renderOrders();
    } catch (err) {
      SB.toast(err.message, { error: true });
      state.orders = [];
      renderOrders();
    }
  }

  function renderOrders() {
    var list = document.getElementById('orderList');
    var secHead = document.getElementById('secHead');
    var empty = document.getElementById('emptyState');
    var countEl = document.getElementById('orderCount');

    var sorted = sortOrders(state.orders.slice(), state.sort);

    if (sorted.length === 0) {
      secHead.style.display = 'none';
      list.innerHTML = '';
      showEmptyState();
      return;
    }

    empty.hidden = true;
    secHead.style.display = 'flex';
    countEl.textContent = sorted.length;

    var html = '';
    for (var i = 0; i < sorted.length; i++) {
      html += buildOrderCard(sorted[i], i);
    }
    list.innerHTML = html;

    list.querySelectorAll('.order-card').forEach(function (card) {
      card.addEventListener('click', function (e) {
        e.preventDefault();
        var oid = parseInt(card.dataset.orderid, 10);
        openOrderDetail(oid);
      });
    });
  }

  function sortOrders(arr, mode) {
    if (mode === 'deadline') {
      arr.sort(function (a, b) {
        return (a.deadline || '').localeCompare(b.deadline || '');
      });
    } else {
      arr.sort(function (a, b) {
        return (b.placedAt || '').localeCompare(a.placedAt || '');
      });
    }
    return arr;
  }

  function buildOrderCard(o, idx) {
    var statusCls = (o.status || '').toLowerCase().replace(/ /g, '_');
    var statusLabel = formatStatus(o.status);
    var delay = Math.min(idx * 0.06, 0.5);

    var html = '<a href="#" class="glass-card interactive order-card" data-orderid="' +
               o.orderID + '" style="animation-delay:' + delay + 's">';

    html += '<div class="order-main">';
    html += '<div class="order-top">';
    html += '<h3>' + SB.escapeHtml(o.gigTitle) + '</h3>';
    html += '<span class="status-badge ' + statusCls + '">';
    html += '<span class="status-dot"></span>' + SB.escapeHtml(statusLabel);
    html += '</span>';
    html += '</div>';

    html += '<div class="order-meta">';
    html += '<span>';
    html += '<svg class="meta-icon" viewBox="0 0 24 24" stroke-linecap="round" stroke-linejoin="round"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>';
    html += SB.escapeHtml(o.otherPartyName);
    html += '</span>';
    if (o.deadline) {
      html += '<span>';
      html += '<svg class="meta-icon" viewBox="0 0 24 24" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><polyline points="12 6 12 12 16 14"/></svg>';
      html += 'Due ' + SB.escapeHtml(o.deadline);
      html += '</span>';
    }
    html += '<span>';
    html += '<svg class="meta-icon" viewBox="0 0 24 24" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="18" rx="2" ry="2"/><line x1="16" y1="2" x2="16" y2="6"/><line x1="8" y1="2" x2="8" y2="6"/><line x1="3" y1="10" x2="21" y2="10"/></svg>';
    html += formatDate(o.placedAt);
    html += '</span>';
    html += '</div>'; 

    html += '</div>'; 

    html += '<div class="order-right">';
    html += '<span class="order-amount">' + SB.formatPrice(o.amount) + '</span>';
    html += '</div>';

    html += '</a>';
    return html;
  }

  async function openOrderDetail(orderID) {
    var overlay = document.getElementById('orderModal');
    var body = document.getElementById('modalBody');
    var actions = document.getElementById('modalActions');
    var title = document.getElementById('modalTitle');

    body.innerHTML = '<div style="padding:20px 0;text-align:center;color:var(--text-faint)">Loading...</div>';
    actions.innerHTML = '';
    title.textContent = 'Order Details';
    overlay.classList.add('show');

    try {
      var data = await SB.api('/api/orders/' + orderID);
      var o = data.order;
      var v = data.viewer;

      title.textContent = 'Order #' + o.orderID;

      var rows = '';
      rows += detailRow('Gig', '<a href="/gig.html?id=' + o.gigID + '">' + SB.escapeHtml(o.gigTitle) + '</a>');
      rows += detailRow('Amount', SB.formatPrice(o.amount));
      rows += detailRow('Status', '<span class="status-badge ' + o.status.toLowerCase().replace(/ /g,'_') + '"><span class="status-dot"></span>' + formatStatus(o.status) + '</span>');

      var partyLabel = v.isBuyer ? 'Seller' : (v.isSeller ? 'Buyer' : 'Other Party');
      rows += detailRow(partyLabel, '<a href="/profile.html?id=' + o.otherPartyID + '">' + SB.escapeHtml(o.otherPartyName) + '</a>');
      rows += detailRow('Deadline', o.deadline || 'N/A');
      rows += detailRow('Placed', formatDate(o.placedAt));
      if (o.completedAt) {
        rows += detailRow('Completed', formatDate(o.completedAt));
      }
      body.innerHTML = rows;

      var btns = '';
      if (v.canAccept) {
        btns += '<button class="btn btn-primary" data-action="IN_PROGRESS">Accept Order</button>';
      }
      if (v.canComplete) {
        btns += '<button class="btn btn-primary" data-action="COMPLETED">Mark Complete</button>';
      }
      if (v.canReview) {
        btns += '<button class="btn btn-primary" id="btnLeaveReview">Leave Review</button>';
      }
      if (v.canCancel) {
        btns += '<button class="btn btn-danger" data-action="CANCELLED">Cancel Order</button>';
      }
      actions.innerHTML = btns;

      var reviewBtn = document.getElementById('btnLeaveReview');
      if (reviewBtn) {
        reviewBtn.addEventListener('click', function () {
          closeModal();
          openReviewModal(orderID);
        });
      }

      actions.querySelectorAll('button[data-action]').forEach(function (btn) {
        btn.addEventListener('click', function () {
          updateOrderStatus(orderID, btn.dataset.action, btn);
        });
      });

    } catch (err) {
      body.innerHTML = '<div style="padding:20px 0;color:var(--error)">' +
                       SB.escapeHtml(err.message) + '</div>';
    }
  }

  async function updateOrderStatus(orderID, newStatus, btn) {
    btn.disabled = true;
    btn.textContent = 'Processing...';

    try {
      var data = await SB.api('/api/orders/' + orderID + '/status', {
        method: 'PATCH',
        body: { status: newStatus }
      });

      if (typeof data.updatedBalance === 'number') {
        var u = SB.getUser();
        if (u) {
          u.balance = data.updatedBalance;
          sessionStorage.setItem('sb_user', JSON.stringify(u));
          SB.renderTopNav({ activeLink: 'orders' });
        }
      }

      SB.toast('Order updated to ' + formatStatus(newStatus));
      closeModal();
      fetchOrders();
    } catch (err) {
      btn.disabled = false;
      btn.textContent = 'Retry';
      SB.toast(err.message, { error: true });
    }
  }

  document.getElementById('modalClose').addEventListener('click', closeModal);
  document.getElementById('orderModal').addEventListener('click', function (e) {
    if (e.target === this) closeModal();
  });
  document.addEventListener('keydown', function (e) {
    if (e.key === 'Escape') closeModal();
  });

  function closeModal() {
    document.getElementById('orderModal').classList.remove('show');
  }

  function detailRow(label, valueHtml) {
    return '<div class="detail-row">' +
           '<span class="detail-label">' + SB.escapeHtml(label) + '</span>' +
           '<span class="detail-value">' + valueHtml + '</span>' +
           '</div>';
  }

  function formatStatus(s) {
    if (!s) return '';
    switch (s) {
      case 'PENDING': return 'Pending';
      case 'IN_PROGRESS': return 'In Progress';
      case 'COMPLETED': return 'Completed';
      case 'CANCELLED': return 'Cancelled';
      default: return s;
    }
  }

  function formatDate(s) {
    if (!s) return '';
    var d = new Date(s.replace(' ', 'T'));
    if (isNaN(d.getTime())) return s;
    return d.toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' });
  }

  function showSkeleton() {
    var list = document.getElementById('orderList');
    document.getElementById('emptyState').hidden = true;
    document.getElementById('secHead').style.display = 'none';
    var html = '';
    for (var i = 0; i < 4; i++) {
      html += '<div class="glass-card skel-order">';
      html += '<div class="so-left">';
      html += '<div class="skeleton so1"></div>';
      html += '<div class="skeleton so2"></div>';
      html += '</div>';
      html += '<div class="so-right">';
      html += '<div class="skeleton so3"></div>';
      html += '<div class="skeleton so4"></div>';
      html += '</div>';
      html += '</div>';
    }
    list.innerHTML = html;
  }

  function showEmptyState() {
    var empty = document.getElementById('emptyState');
    var emptyTitle = document.getElementById('emptyTitle');
    var emptyMsg = document.getElementById('emptyMsg');
    var emptyAction = document.getElementById('emptyAction');

    if (state.tab === 'buyer') {
      emptyTitle.textContent = 'No orders placed yet';
      emptyMsg.textContent = 'Browse gigs and place your first order to get started.';
      emptyAction.innerHTML = '<a href="/browse.html" class="btn btn-primary">Browse gigs</a>';
    } else if (state.tab === 'seller') {
      emptyTitle.textContent = 'No incoming orders';
      emptyMsg.textContent = 'Once clients order your gigs, they will appear here.';
      emptyAction.innerHTML = '';
    } else {
      emptyTitle.textContent = 'No orders in the system';
      emptyMsg.textContent = 'Orders will appear here once users start placing them.';
      emptyAction.innerHTML = '';
    }
    empty.hidden = false;
  }

  var reviewState = { orderID: 0, rating: 0 };

  function openReviewModal(orderID) {
    reviewState.orderID = orderID;
    reviewState.rating = 0;
    document.getElementById('reviewComment').value = '';
    var errBanner = document.getElementById('reviewErrBanner');
    errBanner.textContent = '';
    errBanner.classList.remove('show');

    document.querySelectorAll('#starInput svg').forEach(function (s) {
      s.classList.remove('active');
    });

    document.getElementById('reviewModal').classList.add('show');
    setTimeout(function () {
      try { document.getElementById('reviewComment').focus(); } catch (e) {}
    }, 50);
  }

  function closeReviewModal() {
    document.getElementById('reviewModal').classList.remove('show');
  }

  document.getElementById('starInput').addEventListener('click', function (e) {
    var svg = e.target.closest('svg[data-star]');
    if (!svg) return;
    var val = parseInt(svg.dataset.star, 10);
    reviewState.rating = val;
    document.querySelectorAll('#starInput svg').forEach(function (s) {
      var sv = parseInt(s.dataset.star, 10);
      s.classList.toggle('active', sv <= val);
    });
  });

  document.getElementById('starInput').addEventListener('mouseover', function (e) {
    var svg = e.target.closest('svg[data-star]');
    if (!svg) return;
    var val = parseInt(svg.dataset.star, 10);
    document.querySelectorAll('#starInput svg').forEach(function (s) {
      var sv = parseInt(s.dataset.star, 10);
      s.classList.toggle('active', sv <= val);
    });
  });
  document.getElementById('starInput').addEventListener('mouseleave', function () {
    document.querySelectorAll('#starInput svg').forEach(function (s) {
      var sv = parseInt(s.dataset.star, 10);
      s.classList.toggle('active', sv <= reviewState.rating);
    });
  });

  document.getElementById('reviewClose').addEventListener('click', closeReviewModal);
  document.getElementById('reviewCancelBtn').addEventListener('click', closeReviewModal);
  document.getElementById('reviewModal').addEventListener('click', function (e) {
    if (e.target === this) closeReviewModal();
  });

  document.getElementById('reviewSubmitBtn').addEventListener('click', async function () {
    var errBanner = document.getElementById('reviewErrBanner');
    errBanner.textContent = '';
    errBanner.classList.remove('show');

    if (reviewState.rating < 1 || reviewState.rating > 5) {
      errBanner.textContent = 'Please select a rating (1 to 5 stars).';
      errBanner.classList.add('show');
      return;
    }

    var comment = document.getElementById('reviewComment').value.trim();
    if (!comment) {
      errBanner.textContent = 'Please write a comment.';
      errBanner.classList.add('show');
      return;
    }

    var submitBtn = document.getElementById('reviewSubmitBtn');
    var cancelBtn = document.getElementById('reviewCancelBtn');
    submitBtn.disabled = true;
    cancelBtn.disabled = true;
    submitBtn.textContent = 'Submitting...';

    try {
      await SB.api('/api/orders/' + reviewState.orderID + '/review', {
        method: 'POST',
        body: { rating: reviewState.rating, comment: comment }
      });

      closeReviewModal();
      SB.toast('Review submitted! Sentiment analysis applied automatically.');
      fetchOrders();
    } catch (err) {
      errBanner.textContent = err.message || 'Could not submit review.';
      errBanner.classList.add('show');
    } finally {
      submitBtn.disabled = false;
      cancelBtn.disabled = false;
      submitBtn.textContent = 'Submit Review';
    }
  });

})();