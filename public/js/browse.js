(function () {
  'use strict';

  if (!SB.requireAuth()) return;

  const user = SB.getUser();

  const CATEGORIES = ['DESIGN', 'WRITING', 'CODING', 'MARKETING', 'TUTORING', 'OTHER'];

  const state = {
    category: '',
    sort: 'newest'
  };

  SB.renderTopNav({ activeLink: 'browse' });

  SB.wireSearchBar({
    onAutocomplete: function (prefix, callback) {
      SB.api('/api/gigs/autocomplete?q=' + encodeURIComponent(prefix))
        .then(function (data) { callback(data.suggestions || []); })
        .catch(function () { callback([]); });
    },
    onSubmit: function (q) {
      window.location.href = '/search.html?q=' + encodeURIComponent(q);
    }
  });

  renderPills();

  document.querySelectorAll('.sort-toggle button').forEach(function (b) {
    b.addEventListener('click', function () {
      const newSort = b.dataset.sort;
      if (state.sort === newSort) return;
      state.sort = newSort;
      setActiveSort(newSort);
      refetch();
    });
  });

  refetch();


  function renderPills() {
    const host = document.getElementById('categoryPills');
    const all = makePill('', 'All');
    host.appendChild(all);
    for (const c of CATEGORIES) {
      host.appendChild(makePill(c, prettyCategory(c)));
    }
    setActivePill('');
  }

  function makePill(value, label) {
    const b = document.createElement('button');
    b.type = 'button';
    b.dataset.cat = value;
    b.textContent = label;
    b.setAttribute('role', 'tab');
    b.addEventListener('click', function () {
      if (state.category === value) return;
      state.category = value;
      setActivePill(value);
      refetch();
    });
    return b;
  }

  function setActivePill(value) {
    document.querySelectorAll('#categoryPills button').forEach(function (p) {
      const on = p.dataset.cat === value;
      p.classList.toggle('on', on);
      p.setAttribute('aria-selected', on ? 'true' : 'false');
    });
  }

  function setActiveSort(value) {
    document.querySelectorAll('.sort-toggle button').forEach(function (b) {
      b.classList.toggle('on', b.dataset.sort === value);
    });
  }

  function prettyCategory(c) {
    return c.charAt(0) + c.slice(1).toLowerCase();
  }


  function refetch() {
    showSkeleton();
    hideEmptyState();
    setCount('');

    const params = new URLSearchParams();
    if (state.category) params.set('category', state.category);
    params.set('sort', state.sort);

    SB.api('/api/gigs/browse?' + params.toString())
      .then(function (data) {
        renderResults(data.gigs || [], data.count || 0);
      })
      .catch(function (err) {
        const grid = document.getElementById('gigGrid');
        grid.innerHTML = '';
        grid.setAttribute('aria-busy', 'false');
        SB.toast(err.message || 'Could not load gigs.', { error: true });
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

  function renderResults(gigs, count) {
    const grid = document.getElementById('gigGrid');
    grid.setAttribute('aria-busy', 'false');

    if (gigs.length === 0) {
      grid.innerHTML = '';
      setCount('0');
      showEmptyState();
      return;
    }

    setCount(count);

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

  function setCount(text) {
    document.getElementById('resultCount').textContent = text === '' ? '' : text;
  }

  function showEmptyState() {
    const empty = document.getElementById('emptyState');
    const title = document.getElementById('emptyTitle');
    const msg = document.getElementById('emptyMsg');
    const action = document.getElementById('emptyAction');

    if (state.category) {
      title.textContent = 'No gigs in ' + prettyCategory(state.category);
      msg.textContent = 'Try a different category, or clear the filter to see all gigs.';
      action.innerHTML = '<button type="button" class="btn btn-secondary" id="clearFilterBtn">Show all categories</button>';
      const btn = document.getElementById('clearFilterBtn');
      if (btn) {
        btn.addEventListener('click', function () {
          state.category = '';
          setActivePill('');
          refetch();
        });
      }
    } else if (user.role === 'FREELANCER') {
      title.textContent = 'No gigs yet';
      msg.textContent = 'Be the first to post a gig and start earning.';
      action.innerHTML = '<a href="/gig-edit.html" class="btn btn-primary">Create your first gig</a>';
    } else {
      title.textContent = 'No gigs yet';
      msg.textContent = 'The marketplace is just getting started. Check back soon.';
      action.innerHTML = '';
    }
    empty.hidden = false;
  }

  function hideEmptyState() {
    document.getElementById('emptyState').hidden = true;
  }

})();