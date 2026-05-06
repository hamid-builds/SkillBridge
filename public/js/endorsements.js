(function () {
  'use strict';

  if (!SB.requireAuth()) return;

  SB.renderTopNav({ activeLink: 'endorsements' });

  var $ = function (id) { return document.getElementById(id); };

  var panels = {
    rankings: $('panelRankings'),
    trusted:  $('panelTrusted'),
    given:    $('panelGiven')
  };

  var loaded = { rankings: false, trusted: false, given: false };

  var tabBar = $('tabBar');
  var tabBtns = tabBar.querySelectorAll('.tab-btn');

  tabBar.addEventListener('click', function (e) {
    var btn = e.target.closest('.tab-btn');
    if (!btn) return;
    var tab = btn.dataset.tab;
    if (!tab) return;

    tabBtns.forEach(function (b) { b.classList.remove('active'); });
    btn.classList.add('active');

    Object.keys(panels).forEach(function (k) {
      panels[k].hidden = (k !== tab);
    });

    if (!loaded[tab]) {
      loaded[tab] = true;
      if (tab === 'rankings') loadRankings();
      else if (tab === 'trusted') loadTrusted();
      else if (tab === 'given') loadGiven();
    }
  });

  loaded.rankings = true;
  loadRankings();

  function loadRankings() {
    SB.api('/api/endorsements/rankings?limit=20')
      .then(function (data) {
        $('rankLoading').hidden = true;
        var list = data.rankings || [];
        $('rankCount').textContent = '(' + list.length + ')';

        if (list.length === 0) {
          $('rankEmpty').hidden = false;
          return;
        }

        var maxScore = list[0].score || 1;
        var html = '';
        for (var i = 0; i < list.length; i++) {
          html += rankRowHtml(list[i], maxScore);
        }
        $('rankBody').innerHTML = html;
        $('rankTable').hidden = false;

        $('rankBody').querySelectorAll('.rank-row').forEach(function (row) {
          row.addEventListener('click', function () {
            window.location.href = '/profile.html?id=' + row.dataset.uid;
          });
        });
      })
      .catch(function (err) {
        $('rankLoading').hidden = true;
        SB.toast(err && err.message ? err.message : 'Could not load rankings.', { error: true });
      });
  }

  function rankRowHtml(r, maxScore) {
    var rankClass = '';
    if (r.rank === 1) rankClass = ' gold';
    else if (r.rank === 2) rankClass = ' silver';
    else if (r.rank === 3) rankClass = ' bronze';

    var pct = maxScore > 0 ? Math.round((r.score / maxScore) * 100) : 0;
    var stagger = 'animation-delay:' + (Math.min(r.rank - 1, 12) * 0.04) + 's;';

    return [
      '<tr class="rank-row" data-uid="' + r.userID + '" style="' + stagger + '">',
      '<td><span class="rank-num' + rankClass + '">#' + r.rank + '</span></td>',
      '<td>',
        '<div class="rank-user">',
          '<div class="mini-avatar">' + SB.escapeHtml(SB.initials(r.name)) + '</div>',
          '<span class="name">' + SB.escapeHtml(r.name) + '</span>',
        '</div>',
      '</td>',
      '<td>',
        '<div class="score-bar-wrap">',
          '<div class="score-bar"><div class="score-bar-fill" style="width:' + pct + '%;"></div></div>',
          '<span class="rank-score">' + r.score.toFixed(4) + '</span>',
        '</div>',
      '</td>',
      '</tr>'
    ].join('');
  }

  function loadTrusted() {
    SB.api('/api/endorsements/trusted?maxHops=2')
      .then(function (data) {
        $('trustLoading').hidden = true;
        var list = data.trusted || [];
        $('trustCount').textContent = '(' + list.length + ')';

        if (list.length === 0) {
          $('trustEmpty').hidden = false;
          return;
        }

        var groups = {};
        for (var i = 0; i < list.length; i++) {
          var h = list[i].hopCount;
          if (!groups[h]) groups[h] = [];
          groups[h].push(list[i]);
        }

        var html = '';
        var hops = Object.keys(groups).sort(function (a, b) { return a - b; });
        for (var hi = 0; hi < hops.length; hi++) {
          var hop = hops[hi];
          var label = hop === '1' ? '1 hop away (directly endorsed by you)'
                    : hop + ' hops away';
          html += '<div class="hop-group">';
          html += '<div class="hop-label">' + SB.escapeHtml(label) + '</div>';
          html += '<div class="trust-grid">';
          for (var gi = 0; gi < groups[hop].length; gi++) {
            html += trustCardHtml(groups[hop][gi], gi);
          }
          html += '</div></div>';
        }

        var content = $('trustContent');
        content.innerHTML = html;
        content.hidden = false;

        content.querySelectorAll('.trust-card').forEach(function (card) {
          card.addEventListener('click', function () {
            window.location.href = '/profile.html?id=' + card.dataset.uid;
          });
        });
      })
      .catch(function (err) {
        $('trustLoading').hidden = true;
        SB.toast(err && err.message ? err.message : 'Could not load trust network.', { error: true });
      });
  }

  function trustCardHtml(t, idx) {
    var stagger = 'animation-delay:' + (Math.min(idx, 10) * 0.04) + 's;';
    return [
      '<div class="glass-card trust-card" tabindex="0" role="button" data-uid="' + t.userID + '" style="' + stagger + '">',
        '<div class="mini-avatar">' + SB.escapeHtml(SB.initials(t.name)) + '</div>',
        '<div>',
          '<div class="name">' + SB.escapeHtml(t.name) + '</div>',
          '<div class="hop-tag">' + t.hopCount + ' hop' + (t.hopCount === 1 ? '' : 's') + '</div>',
        '</div>',
      '</div>'
    ].join('');
  }

  function loadGiven() {
    SB.api('/api/endorsements/given')
      .then(function (data) {
        $('givenLoading').hidden = true;
        var list = data.endorsements || [];
        $('givenCount').textContent = '(' + list.length + ')';

        if (list.length === 0) {
          $('givenEmpty').hidden = false;
          return;
        }

        renderGivenList(list);
      })
      .catch(function (err) {
        $('givenLoading').hidden = true;
        SB.toast(err && err.message ? err.message : 'Could not load endorsements.', { error: true });
      });
  }

  function renderGivenList(list) {
    var container = $('givenList');
    var html = '';
    for (var i = 0; i < list.length; i++) {
      html += givenCardHtml(list[i], i);
    }
    container.innerHTML = html;
    container.hidden = false;

    container.querySelectorAll('.btn-remove').forEach(function (btn) {
      btn.addEventListener('click', function (e) {
        e.stopPropagation();
        var eid = parseInt(btn.dataset.eid, 10);
        removeEndorsement(eid, btn);
      });
    });
  }

  function givenCardHtml(e, idx) {
    var stagger = 'animation-delay:' + (Math.min(idx, 10) * 0.04) + 's;';
    var weightStr = e.weight !== 1 ? ' (weight ' + e.weight.toFixed(1) + ')' : '';
    return [
      '<div class="glass-card given-card" style="' + stagger + '">',
        '<div class="mini-avatar">' + SB.escapeHtml(SB.initials(e.toUserName)) + '</div>',
        '<div class="info">',
          '<div class="top-line">',
            '<a href="/profile.html?id=' + e.toUserID + '">' + SB.escapeHtml(e.toUserName) + '</a>',
          '</div>',
          '<div class="bottom-line">' + SB.escapeHtml(formatDate(e.timestamp)) + weightStr + '</div>',
        '</div>',
        '<span class="skill-chip">' + SB.escapeHtml(e.skill) + '</span>',
        '<button class="btn-remove" data-eid="' + e.endorsementID + '">Remove</button>',
      '</div>'
    ].join('');
  }

  function removeEndorsement(endorsementID, btn) {
    var orig = btn.textContent;
    btn.disabled = true;
    btn.textContent = '...';

    SB.api('/api/endorsements/' + endorsementID, { method: 'DELETE' })
      .then(function () {
        SB.toast('Endorsement removed.');
        var card = btn.closest('.given-card');
        if (card) card.remove();
        var remaining = $('givenList').querySelectorAll('.given-card').length;
        $('givenCount').textContent = '(' + remaining + ')';
        if (remaining === 0) {
          $('givenList').hidden = true;
          $('givenEmpty').hidden = false;
        }
        loaded.rankings = false;
        loaded.trusted = false;
      })
      .catch(function (err) {
        btn.disabled = false;
        btn.textContent = orig;
        SB.toast(err && err.message ? err.message : 'Could not remove endorsement.', { error: true });
      });
  }

  function formatDate(ts) {
    if (!ts) return '';
    var dPart = String(ts).split(' ')[0];
    var m = /^(\d{4})-(\d{2})-(\d{2})$/.exec(dPart);
    if (!m) return ts;
    var months = ['Jan','Feb','Mar','Apr','May','Jun',
                  'Jul','Aug','Sep','Oct','Nov','Dec'];
    var month = months[parseInt(m[2], 10) - 1] || '';
    var day = parseInt(m[3], 10);
    return month + ' ' + day + ', ' + m[1];
  }

})();