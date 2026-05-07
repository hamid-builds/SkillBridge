(function () {
  'use strict';

  if (!SB.requireAuth()) return;

  var me = SB.getUser();

  if (me.role !== 'FREELANCER') {
    window.location.href = '/browse.html';
    return;
  }

  SB.renderTopNav({ activeLink: 'mygigs' });

  var gigGrid = document.getElementById('gigGrid');
  var gigCount = document.getElementById('gigCount');
  var emptyState = document.getElementById('emptyState');

  loadMyGigs();

  function loadMyGigs() {
    gigGrid.setAttribute('aria-busy', 'true');
    var skelHtml = '';
    for (var i = 0; i < 3; i++) {
      skelHtml +=
        '<div class="glass-card skel-card">' +
          '<div class="skeleton s1"></div>' +
          '<div class="skeleton s2"></div>' +
          '<div class="skeleton s3"></div>' +
          '<div class="skeleton s4"></div>' +
          '<div class="s5"><div class="skeleton a"></div><div class="skeleton n"></div><div class="skeleton p"></div></div>' +
        '</div>';
    }
    gigGrid.innerHTML = skelHtml;

    SB.api('/api/gigs/mine')
      .then(function (data) {
        gigGrid.setAttribute('aria-busy', 'false');
        var gigs = data.gigs || [];
        gigCount.textContent = gigs.length;

        if (gigs.length === 0) {
          gigGrid.innerHTML = '';
          emptyState.hidden = false;
          return;
        }

        emptyState.hidden = true;
        renderGigs(gigs);
      })
      .catch(function (err) {
        gigGrid.setAttribute('aria-busy', 'false');
        gigGrid.innerHTML = '';
        SB.toast(err && err.message ? err.message : 'Could not load gigs.', { error: true });
      });
  }

  function renderGigs(gigs) {
    var html = '';
    for (var i = 0; i < gigs.length; i++) {
      html += gigCardHtml(gigs[i], i);
    }
    gigGrid.innerHTML = html;
    wireCardActions();
  }

  function gigCardHtml(g, idx) {
    var stagger = 'animation-delay:' + (Math.min(idx, 8) * 0.04) + 's;';
    var isActive = g.isActive !== false;
    var inactiveClass = isActive ? '' : ' inactive-card';
    var pillClass = isActive ? 'active' : 'inactive';
    var pillLabel = isActive ? 'Active' : 'Inactive';
    var toggleLabel = isActive ? 'Deactivate' : 'Activate';

    var cat = prettyCategory(g.category);
    var price = SB.formatPrice(g.price);
    var desc = g.shortDescription || '';

    return [
      '<div class="glass-card gig-card' + inactiveClass + '" style="' + stagger + '" data-gigid="' + g.gigID + '">',
        '<div style="display:flex;justify-content:space-between;align-items:center;">',
          '<div class="cat">' + SB.escapeHtml(cat) + '</div>',
          '<span class="status-pill ' + pillClass + '"><span class="dot"></span>' + pillLabel + '</span>',
        '</div>',
        '<h3>' + SB.escapeHtml(g.title) + '</h3>',
        '<div class="desc">' + SB.escapeHtml(desc) + '</div>',
        '<div class="foot">',
          '<div class="seller">',
            '<div class="mini-avatar">' + SB.escapeHtml(SB.initials(g.freelancerName)) + '</div>',
            '<div class="seller-info">',
              '<div class="seller-name">' + SB.escapeHtml(g.freelancerName || 'You') + '</div>',
              ratingHtml(g.freelancerAvgRating),
            '</div>',
          '</div>',
          '<div class="price">' + SB.escapeHtml(price) + '</div>',
        '</div>',
        '<div class="card-actions">',
          '<button class="act-edit" data-action="edit" data-gid="' + g.gigID + '">Edit</button>',
          '<button class="act-toggle" data-action="toggle" data-gid="' + g.gigID + '" data-active="' + (isActive ? '1' : '0') + '">' + toggleLabel + '</button>',
        '</div>',
      '</div>'
    ].join('');
  }

  function ratingHtml(avg) {
    if (avg > 0) {
      return '<div class="seller-rating">' +
        '<svg class="star" viewBox="0 0 24 24"><polygon points="12 2 15 9 22 9.5 17 14.5 18.5 22 12 18 5.5 22 7 14.5 2 9.5 9 9"/></svg>' +
        '<span>' + Number(avg).toFixed(1) + '</span></div>';
    }
    return '<div class="seller-rating"><span>New seller</span></div>';
  }

  function wireCardActions() {
    gigGrid.querySelectorAll('[data-action="edit"]').forEach(function (btn) {
      btn.addEventListener('click', function (e) {
        e.stopPropagation();
        window.location.href = '/gig-edit.html?id=' + btn.dataset.gid;
      });
    });

    gigGrid.querySelectorAll('[data-action="toggle"]').forEach(function (btn) {
      btn.addEventListener('click', function (e) {
        e.stopPropagation();
        var gigID = btn.dataset.gid;
        var currentlyActive = btn.dataset.active === '1';
        var newActive = !currentlyActive;

        btn.disabled = true;
        btn.textContent = newActive ? 'Activating...' : 'Deactivating...';

        SB.api('/api/gigs/' + gigID + '/active', {
          method: 'PATCH',
          body: { isActive: newActive }
        })
          .then(function () {
            SB.toast(newActive ? 'Gig activated.' : 'Gig deactivated.');
            loadMyGigs();
          })
          .catch(function (err) {
            SB.toast(err && err.message ? err.message : 'Could not update gig.', { error: true });
            btn.disabled = false;
            btn.textContent = currentlyActive ? 'Deactivate' : 'Activate';
          });
      });
    });

    gigGrid.querySelectorAll('.gig-card').forEach(function (card) {
      card.style.cursor = 'pointer';
      card.addEventListener('click', function (e) {
        if (e.target.closest('[data-action]')) return;
        window.location.href = '/gig.html?id=' + card.dataset.gigid;
      });
    });
  }

  function prettyCategory(c) {
    if (!c) return '';
    return c.charAt(0) + c.slice(1).toLowerCase();
  }

})();