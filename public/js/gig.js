(function () {
  'use strict';

  if (!SB.requireAuth()) return;

  function readGigIdFromUrl() {
    const params = new URLSearchParams(window.location.search);
    const raw = params.get('id') || '';
    const trimmed = raw.trim();
    if (!trimmed) return null;
    if (!/^\d+$/.test(trimmed)) return null;
    const n = parseInt(trimmed, 10);
    if (!isFinite(n) || n <= 0) return null;
    return n;
  }

  const gigID = readGigIdFromUrl();
  if (gigID === null) {
    window.location.replace('/browse.html');
    return;
  }

  SB.renderTopNav({ activeLink: 'browse' });

  const $loading = document.getElementById('loadingState');
  const $content = document.getElementById('contentRoot');
  const $notFound = document.getElementById('notFoundState');
  const $closedBanner = document.getElementById('closedBanner');
  const $gigCategory = document.getElementById('gigCategory');
  const $gigTitle = document.getElementById('gigTitle');
  const $gigPosted = document.getElementById('gigPosted');
  const $gigDesc = document.getElementById('gigDesc');
  const $gigPrice = document.getElementById('gigPrice');
  const $sellerAvatar = document.getElementById('sellerAvatar');
  const $sellerName = document.getElementById('sellerName');
  const $sellerRating = document.getElementById('sellerRating');
  const $sellerSkills = document.getElementById('sellerSkills');
  const $sellerProfileLink = document.getElementById('sellerProfileLink');
  const $actionButtons = document.getElementById('actionButtons');
  const $reviewCount = document.getElementById('reviewCount');
  const $seeAllReviews = document.getElementById('seeAllReviews');
  const $reviewsList = document.getElementById('reviewsList');
  const $reviewsEmpty = document.getElementById('reviewsEmpty');

  loadPage();

  function loadPage() {
    SB.api('/api/gigs/' + gigID)
      .then(function (gigData) {
        renderGig(gigData);
        const freelancerID = gigData.freelancer.userID;
        return SB.api('/api/users/' + freelancerID + '/reviews?limit=3')
          .then(function (reviewData) {
            renderReviews(reviewData, freelancerID);
          })
          .catch(function (err) {
            renderReviews({ reviews: [], totalCount: 0 }, freelancerID);
            SB.toast('Could not load reviews.', { error: true });
          });
      })
      .catch(function (err) {
        if (err && err.status === 404) {
          showNotFound();
          return;
        }
        $loading.hidden = true;
        $content.hidden = true;
        SB.toast(err && err.message ? err.message : 'Could not load this gig.', { error: true });
      });
  }


  function renderGig(data) {
    const g = data.gig;
    const f = data.freelancer;
    const v = data.viewer;

    $gigCategory.textContent = prettyCategory(g.category);
    $gigTitle.textContent = g.title;
    $gigPosted.textContent = 'Posted on ' + formatDate(g.createdAt);

    if (g.isActive === false) {
      $closedBanner.hidden = false;
    }

    $gigDesc.textContent = g.description || '';

    $gigPrice.textContent = SB.formatPrice(g.price);

    $sellerAvatar.textContent = SB.initials(f.name);
    $sellerName.textContent = f.name || 'Unknown';
    $sellerRating.innerHTML = renderSellerRating(f.avgRating, f.reviewCount);
    $sellerSkills.textContent = f.skills && f.skills.trim()
      ? f.skills
      : 'No skills listed yet.';
    $sellerProfileLink.href = '/profile.html?id=' + encodeURIComponent(f.userID);

    renderActionButtons(g, f, v);

    $loading.hidden = true;
    $content.hidden = false;
  }

  function renderSellerRating(avg, count) {
    if (!count || count === 0 || !avg || avg <= 0) {
      return '<span>No reviews yet</span>';
    }
    const star = '<svg class="star" viewBox="0 0 24 24"><polygon points="12 2 15 9 22 9.5 17 14.5 18.5 22 12 18 5.5 22 7 14.5 2 9.5 9 9"/></svg>';
    return star + '<span>' + Number(avg).toFixed(1) + ' (' + count + ' review' + (count === 1 ? '' : 's') + ')</span>';
  }

  function renderActionButtons(g, f, v) {
    const out = [];


    if (v.canDelete) {
      out.push(
        '<button type="button" class="btn btn-danger" data-act="delete">' +
        'Delete gig</button>'
      );
    } else if (v.canEdit) {
      if (g.isActive === false) {
        out.push(
          '<button type="button" class="btn btn-primary" data-act="activate">' +
          'Reactivate gig</button>'
        );
        out.push(
          '<button type="button" class="btn btn-secondary" data-act="edit">' +
          'Edit gig</button>'
        );
      } else {
        out.push(
          '<button type="button" class="btn btn-primary" data-act="edit">' +
          'Edit gig</button>'
        );
      }
    } else if (v.canOrder) {
      out.push(
        '<button type="button" class="btn btn-primary" data-act="order">' +
        'Order this gig</button>'
      );
    }

    if (!v.isOwner) {
      out.push(
        '<button type="button" class="btn btn-secondary" data-act="message">' +
        'Message seller</button>'
      );
    }

    if (v.canEdit && g.isActive !== false) {
      out.push(
        '<button type="button" class="deact-link" data-act="deactivate">' +
        'Deactivate this gig</button>'
      );
    }

    if (out.length === 0) {
      out.push('<div class="no-actions">No actions available for your account on this gig.</div>');
    }

    $actionButtons.innerHTML = out.join('');

    $actionButtons.querySelectorAll('[data-act]').forEach(function (btn) {
      btn.addEventListener('click', function () {
        const act = btn.dataset.act;
        handleAction(act, g, f, v, btn);
      });
    });
  }

  function handleAction(act, g, f, v, btn) {
    switch (act) {
      case 'order':
        openPlaceOrderModal(g, f);
        return;

      case 'message':
        window.location.href = '/messages.html?with=' + encodeURIComponent(f.userID);
        return;

      case 'edit':
        window.location.href = '/gig-edit.html?id=' + encodeURIComponent(g.gigID);
        return;

      case 'deactivate':
        confirmAndToggleActive(g.gigID, false, btn);
        return;

      case 'activate':
        toggleActive(g.gigID, true, btn);
        return;

      case 'delete':
        confirmAndDelete(g.gigID, btn);
        return;
    }
  }

  function confirmAndToggleActive(id, active, btn) {
    if (!active) {
      const ok = window.confirm(
        'Pause this gig? It will stop appearing in browse and search until you reactivate it.'
      );
      if (!ok) return;
    }
    toggleActive(id, active, btn);
  }

  function toggleActive(id, active, btn) {
    const wasText = btn.textContent;
    btn.disabled = true;
    btn.textContent = active ? 'Reactivating...' : 'Pausing...';

    SB.api('/api/gigs/' + id + '/active', {
      method: 'PATCH',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ isActive: active })
    })
      .then(function () {
        SB.toast(active ? 'Gig reactivated.' : 'Gig paused.');
        setTimeout(function () {
          window.location.reload();
        }, 500);
      })
      .catch(function (err) {
        btn.disabled = false;
        btn.textContent = wasText;
        SB.toast(err && err.message ? err.message : 'Could not change gig status.', { error: true });
      });
  }

  function confirmAndDelete(id, btn) {
    const ok = window.confirm(
      'Permanently delete this gig? This cannot be undone.'
    );
    if (!ok) return;

    btn.disabled = true;
    btn.textContent = 'Deleting...';

    SB.api('/api/gigs/' + id, { method: 'DELETE' })
      .then(function () {
        SB.toast('Gig deleted.');
        setTimeout(function () {
          window.location.href = '/browse.html';
        }, 600);
      })
      .catch(function (err) {
        btn.disabled = false;
        btn.textContent = 'Delete gig';
        SB.toast(err && err.message ? err.message : 'Could not delete gig.', { error: true });
      });
  }

  function renderReviews(data, freelancerID) {
    const list = data.reviews || [];
    const total = data.totalCount || 0;

    $reviewCount.textContent = total > 0 ? '(' + total + ')' : '';

    if (total === 0) {
      $reviewsList.innerHTML = '';
      $reviewsEmpty.hidden = false;
      $seeAllReviews.hidden = true;
      return;
    }

    if (total > list.length) {
      $seeAllReviews.hidden = false;
      $seeAllReviews.href = '/profile.html?id=' + encodeURIComponent(freelancerID);
    } else {
      $seeAllReviews.hidden = true;
    }

    let html = '';
    for (let i = 0; i < list.length; i++) {
      html += reviewCardHtml(list[i], i);
    }
    $reviewsList.innerHTML = html;
    $reviewsEmpty.hidden = true;
  }

  function reviewCardHtml(r, idx) {
    const stagger = 'animation-delay:' + (Math.min(idx, 8) * 0.05) + 's;';
    const stars = renderStars(r.rating);
    const initials = SB.initials(r.reviewerName);
    const name = SB.escapeHtml(r.reviewerName || 'Unknown');
    const date = SB.escapeHtml(formatDate(r.createdAt));

    return [
      '<div class="glass-card review-card" style="' + stagger + '">',
      '<div class="stars-row">' + stars + '</div>',
      '<div class="comment">' + SB.escapeHtml(r.comment || '') + '</div>',
      '<div class="meta">',
      '<div class="mini-avatar">' + SB.escapeHtml(initials) + '</div>',
      '<div class="reviewer-name">' + name + '</div>',
      '<div class="ts">' + date + '</div>',
      '</div>',
      '</div>'
    ].join('');
  }

  function renderStars(rating) {
    const r = Math.max(0, Math.min(5, parseInt(rating, 10) || 0));
    let html = '';
    for (let i = 1; i <= 5; i++) {
      const cls = i <= r ? 'filled' : 'empty';
      html += '<svg class="' + cls + '" viewBox="0 0 24 24"><polygon points="12 2 15 9 22 9.5 17 14.5 18.5 22 12 18 5.5 22 7 14.5 2 9.5 9 9"/></svg>';
    }
    return html;
  }


  function prettyCategory(c) {
    if (!c) return '';
    return c.charAt(0) + c.slice(1).toLowerCase();
  }

  function formatDate(ts) {
    if (!ts) return '';
    const dPart = String(ts).split(' ')[0];
    const m = /^(\d{4})-(\d{2})-(\d{2})$/.exec(dPart);
    if (!m) return ts;
    const months = ['Jan','Feb','Mar','Apr','May','Jun',
                    'Jul','Aug','Sep','Oct','Nov','Dec'];
    const month = months[parseInt(m[2], 10) - 1] || '';
    const day = parseInt(m[3], 10);
    return month + ' ' + day + ', ' + m[1];
  }

  function showNotFound() {
    $loading.hidden = true;
    $content.hidden = true;
    $notFound.hidden = false;
  }

  function openPlaceOrderModal(g, f) {
    // Remove any existing modal
    var existing = document.getElementById('placeOrderOverlay');
    if (existing) existing.remove();

    var user = SB.getUser();
    var balance = (user && typeof user.balance === 'number') ? user.balance : 0;
    var canAfford = balance >= g.price;

    var overlay = document.createElement('div');
    overlay.id = 'placeOrderOverlay';
    overlay.style.cssText = 'position:fixed;inset:0;background:rgba(31,42,38,0.3);backdrop-filter:blur(4px);-webkit-backdrop-filter:blur(4px);z-index:80;display:grid;place-items:center;animation:fadeUp .2s ease-out;';

    // Default deadline: 7 days from now
    var defDate = new Date();
    defDate.setDate(defDate.getDate() + 7);
    var defStr = defDate.toISOString().split('T')[0];

    // Min deadline: tomorrow
    var minDate = new Date();
    minDate.setDate(minDate.getDate() + 1);
    var minStr = minDate.toISOString().split('T')[0];

    var modalHtml = '';
    modalHtml += '<div style="background:var(--bg-1);border:1px solid var(--line);border-radius:18px;box-shadow:0 24px 64px rgba(31,42,38,0.18);width:90%;max-width:440px;padding:28px;">';

    modalHtml += '<div style="display:flex;justify-content:space-between;align-items:flex-start;margin-bottom:20px;">';
    modalHtml += '<h2 style="font-size:20px;font-weight:600;letter-spacing:-0.02em;">Place Order</h2>';
    modalHtml += '<button id="poClose" style="width:32px;height:32px;display:grid;place-items:center;border-radius:8px;color:var(--text-dim);cursor:pointer;border:none;background:transparent;" aria-label="Close"><svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg></button>';
    modalHtml += '</div>';

    // Gig summary
    modalHtml += '<div style="padding:14px;background:var(--accent-soft);border-radius:12px;margin-bottom:20px;">';
    modalHtml += '<div style="font-size:11px;font-family:Geist Mono,monospace;color:var(--accent);text-transform:uppercase;letter-spacing:0.12em;margin-bottom:4px;">' + SB.escapeHtml(prettyCategory(g.category)) + '</div>';
    modalHtml += '<div style="font-size:15px;font-weight:600;color:var(--text);margin-bottom:2px;">' + SB.escapeHtml(g.title) + '</div>';
    modalHtml += '<div style="font-size:12px;color:var(--text-dim);">by ' + SB.escapeHtml(f.name) + '</div>';
    modalHtml += '</div>';

    // Amount row
    modalHtml += '<div style="display:flex;justify-content:space-between;align-items:center;padding:10px 0;border-bottom:1px solid var(--line);font-size:13px;">';
    modalHtml += '<span style="color:var(--text-dim);font-weight:500;">Amount</span>';
    modalHtml += '<span style="font-family:Geist Mono,monospace;font-size:16px;font-weight:600;color:var(--text);">' + SB.formatPrice(g.price) + '</span>';
    modalHtml += '</div>';

    // Balance row
    modalHtml += '<div style="display:flex;justify-content:space-between;align-items:center;padding:10px 0;border-bottom:1px solid var(--line);font-size:13px;">';
    modalHtml += '<span style="color:var(--text-dim);font-weight:500;">Your balance</span>';
    modalHtml += '<span style="font-family:Geist Mono,monospace;font-size:14px;font-weight:500;color:' + (canAfford ? 'var(--accent)' : 'var(--error)') + ';">' + SB.formatPrice(balance) + '</span>';
    modalHtml += '</div>';

    // Deadline field
    modalHtml += '<div class="field" style="margin-top:16px;">';
    modalHtml += '<label for="poDeadline">Deadline</label>';
    modalHtml += '<input type="date" id="poDeadline" value="' + defStr + '" min="' + minStr + '">';
    modalHtml += '</div>';

    // Error area
    modalHtml += '<div id="poError" class="error-banner" style="margin-top:12px;"></div>';

    // Buttons
    modalHtml += '<div style="display:flex;gap:10px;margin-top:20px;">';
    if (canAfford) {
      modalHtml += '<button id="poSubmit" class="btn btn-primary" style="flex:1;">Confirm Order</button>';
    } else {
      modalHtml += '<button class="btn btn-primary" disabled style="flex:1;opacity:0.5;cursor:not-allowed;">Insufficient balance</button>';
    }
    modalHtml += '<button id="poCancel" class="btn btn-secondary">Cancel</button>';
    modalHtml += '</div>';

    modalHtml += '</div>';

    overlay.innerHTML = modalHtml;
    document.body.appendChild(overlay);

    var closeFn = function () { overlay.remove(); };
    document.getElementById('poClose').addEventListener('click', closeFn);
    document.getElementById('poCancel').addEventListener('click', closeFn);
    overlay.addEventListener('click', function (e) {
      if (e.target === overlay) closeFn();
    });

    var escFn = function (e) {
      if (e.key === 'Escape') {
        closeFn();
        document.removeEventListener('keydown', escFn);
      }
    };
    document.addEventListener('keydown', escFn);

    if (!canAfford) return;

    document.getElementById('poSubmit').addEventListener('click', function () {
      var submitBtn = document.getElementById('poSubmit');
      var deadlineVal = document.getElementById('poDeadline').value;
      var errEl = document.getElementById('poError');

      if (!deadlineVal) {
        errEl.textContent = 'Please select a deadline.';
        errEl.classList.add('show');
        return;
      }

      submitBtn.disabled = true;
      submitBtn.textContent = 'Placing order...';
      errEl.classList.remove('show');

      SB.api('/api/orders', {
        method: 'POST',
        body: { gigID: g.gigID, deadline: deadlineVal }
      })
        .then(function (data) {
          if (typeof data.updatedBalance === 'number') {
            var u = SB.getUser();
            if (u) {
              u.balance = data.updatedBalance;
              sessionStorage.setItem('sb_user', JSON.stringify(u));
            }
          }
          closeFn();
          SB.toast('Order placed successfully!');
          setTimeout(function () {
            window.location.href = '/orders.html';
          }, 800);
        })
        .catch(function (err) {
          submitBtn.disabled = false;
          submitBtn.textContent = 'Confirm Order';
          errEl.textContent = err.message || 'Something went wrong.';
          errEl.classList.add('show');
        });
    });
  }

})();