(function () {
  'use strict';

  if (!SB.requireAuth()) return;

  function readQueryFromUrl() {
    const params = new URLSearchParams(window.location.search);
    const raw = params.get('q') || '';
    return raw.trim();
  }

  let currentQuery = readQueryFromUrl();
  if (!currentQuery) {
    window.location.replace('/browse.html');
    return;
  }

  SB.renderTopNav({ activeLink: 'browse' });

  const searchInput = document.getElementById('search-input');
  searchInput.value = currentQuery;

  SB.wireSearchBar({
    onAutocomplete: function (prefix, callback) {
      SB.api('/api/gigs/autocomplete?q=' + encodeURIComponent(prefix))
        .then(function (data) { callback(data.suggestions || []); })
        .catch(function () { callback([]); });
    },
    onSubmit: function (q) {
      const trimmed = (q || '').trim();
      if (!trimmed) return;
      if (trimmed === currentQuery) return;

      const newUrl = '/search.html?q=' + encodeURIComponent(trimmed);
      window.history.pushState({ q: trimmed }, '', newUrl);
      currentQuery = trimmed;
      runSearch(currentQuery);
    }
  });

  window.addEventListener('popstate', function () {
    const q = readQueryFromUrl();
    if (!q) {
      window.location.replace('/browse.html');
      return;
    }
    currentQuery = q;
    searchInput.value = q;
    runSearch(q);
  });

  runSearch(currentQuery);


  function runSearch(q) {
    showSkeleton();
    hideEmptyState();
    setHeader(q, '');

    SB.api('/api/gigs/search?q=' + encodeURIComponent(q))
      .then(function (data) {
        renderResults(data.results || [], data.count || 0, q);
      })
      .catch(function (err) {
        const grid = document.getElementById('gigGrid');
        grid.innerHTML = '';
        grid.setAttribute('aria-busy', 'false');
        SB.toast(err.message || 'Could not run search.', { error: true });
      });
  }

  function showSkeleton() {
    const grid = document.getElementById('gigGrid');
    grid.setAttribute('aria-busy', 'true');
    let html = '';
    for (let i = 0; i < 6; i++) html += skeletonCardHtml();
    grid.innerHTML = html;
  }

  function skeletonCardHtml() {
    return [
      '<div class="glass-card skel-card">',
      '<div class="skeleton s1"></div>',
      '<div class="skeleton s2"></div>',
      '<div class="skeleton s3"></div>',
      '<div class="skeleton s4"></div>',
      '<div class="s5">',
      '<div class="skeleton a"></div>',
      '<div class="skeleton n"></div>',
      '<div class="skeleton p"></div>',
      '</div>',
      '</div>'
    ].join('');
  }

  function renderResults(gigs, count, q) {
    const grid = document.getElementById('gigGrid');
    grid.setAttribute('aria-busy', 'false');

    if (gigs.length === 0) {
      grid.innerHTML = '';
      setHeader(q, 0);
      showEmptyState(q);
      return;
    }

    setHeader(q, count);

    let html = '';
    for (let i = 0; i < gigs.length; i++) {
      html += gigCardHtml(gigs[i], i);
    }
    grid.innerHTML = html;

    grid.querySelectorAll('[data-gigid]').forEach(function (el) {
      el.addEventListener('click', function () {
        window.location.href = '/gig.html?id=' + encodeURIComponent(el.dataset.gigid);
      });
      el.addEventListener('keydown', function (e) {
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          el.click();
        }
      });
    });
  }

  function gigCardHtml(g, idx) {
    const initials = SB.initials(g.freelancerName);
    const stagger = 'animation-delay:' + (Math.min(idx, 8) * 0.04) + 's;';
    const ratingLine = (g.freelancerAvgRating > 0)
      ? '<div class="seller-rating">' + starSvg() +
        '<span>' + g.freelancerAvgRating.toFixed(1) + '</span></div>'
      : '<div class="seller-rating"><span>New seller</span></div>';

    return [
      '<a class="glass-card interactive gig-card" tabindex="0"',
      ' role="button" data-gigid="' + g.gigID + '" style="' + stagger + '">',
      '<div class="cat">' + SB.escapeHtml(prettyCategory(g.category)) + '</div>',
      '<h3>' + SB.escapeHtml(g.title) + '</h3>',
      '<div class="desc">' + SB.escapeHtml(g.shortDescription || '') + '</div>',
      '<div class="foot">',
      '<div class="seller">',
      '<div class="mini-avatar">' + SB.escapeHtml(initials) + '</div>',
      '<div class="seller-info">',
      '<div class="seller-name">' + SB.escapeHtml(g.freelancerName || 'Unknown') + '</div>',
      ratingLine,
      '</div>',
      '</div>',
      '<div class="price">' + SB.escapeHtml(SB.formatPrice(g.price)) + '</div>',
      '</div>',
      '</a>'
    ].join('');
  }

  function starSvg() {
    return '<svg class="star" viewBox="0 0 24 24"><polygon points="12 2 15 9 22 9.5 17 14.5 18.5 22 12 18 5.5 22 7 14.5 2 9.5 9 9"/></svg>';
  }

  function prettyCategory(c) {
    if (!c) return '';
    return c.charAt(0) + c.slice(1).toLowerCase();
  }

  function setHeader(q, count) {
    const titleEl = document.getElementById('resultTitle');
    const countEl = document.getElementById('resultCount');
    titleEl.textContent = 'Results for "' + q + '"';
    countEl.textContent = (count === '' || count === null || count === undefined)
      ? ''
      : String(count);
  }

  function showEmptyState(q) {
    const empty = document.getElementById('emptyState');
    const title = document.getElementById('emptyTitle');
    const msg = document.getElementById('emptyMsg');
    const action = document.getElementById('emptyAction');

    title.textContent = 'No gigs match your search';
    msg.textContent = 'No results for "' + q + '". Try different keywords or browse all gigs.';
    action.innerHTML = '<a href="/browse.html" class="btn btn-secondary">Browse all gigs</a>';
    empty.hidden = false;
  }

  function hideEmptyState() {
    document.getElementById('emptyState').hidden = true;
  }

})();