(function () {
  'use strict';

  if (!SB.requireAuth()) return;

  const me = SB.getUser();

  function readIdFromUrl() {
    const params = new URLSearchParams(window.location.search);
    const raw = (params.get('id') || '').trim();
    if (!raw) return null;
    if (!/^\d+$/.test(raw)) return null;
    const n = parseInt(raw, 10);
    if (!isFinite(n) || n <= 0) return null;
    return n;
  }

  const targetID = readIdFromUrl();
  const isOwnProfile = (targetID === null) || (targetID === me.userID);
  const fetchID = isOwnProfile ? me.userID : targetID;

  SB.renderTopNav({ activeLink: 'browse' });

  const $ = (id) => document.getElementById(id);
  const els = {
    loading: $('loadingState'),
    content: $('contentRoot'),
    notFound: $('notFoundState'),
    backLink: $('backLink'),
    backLabel: $('backLabel'),

    hAvatar: $('hAvatar'),
    hName: $('hName'),
    hRole: $('hRole'),
    hJoined: $('hJoined'),
    hRating: $('hRating'),

    flSec: $('freelancerSections'),
    portfolioBody: $('portfolioBody'),
    skillsBody: $('skillsBody'),

    gigsCount: $('gigsCount'),
    gigsGrid: $('gigsGrid'),
    gigsEmpty: $('gigsEmpty'),
    gigsEmptyTitle: $('gigsEmptyTitle'),
    gigsEmptyMsg: $('gigsEmptyMsg'),
    gigsEmptyAction: $('gigsEmptyAction'),

    reviewsCount: $('reviewsCount'),
    reviewsList: $('reviewsList'),
    reviewsEmpty: $('reviewsEmpty'),

    acSec: $('accountSection'),
    acName: $('acName'),
    acPortfolioWrap: $('acPortfolioWrap'),
    acPortfolio: $('acPortfolio'),
    acSkillsWrap: $('acSkillsWrap'),
    acSkills: $('acSkills'),
    profileErrBanner: $('profileErrBanner'),
    btnSaveProfile: $('btnSaveProfile'),

    pwOld: $('pwOld'),
    pwNew: $('pwNew'),
    pwErrBanner: $('pwErrBanner'),
    btnChangePw: $('btnChangePw'),

    btnDelete: $('btnDelete'),

    endorseCount: $('endorseCount'),
    endorseList: $('endorseList'),
    endorseEmpty: $('endorseEmpty'),
    btnEndorse: $('btnEndorse'),

    endorseModal: $('endorseModal'),
    emSkill: $('emSkill'),
    emWeight: $('emWeight'),
    emWeightVal: $('emWeightVal'),
    emErrBanner: $('emErrBanner'),
    emCancel: $('emCancel'),
    emConfirm: $('emConfirm'),

    deleteModal: $('deleteModal'),
    dmPassword: $('dmPassword'),
    dmErrBanner: $('dmErrBanner'),
    dmCancel: $('dmCancel'),
    dmConfirm: $('dmConfirm')
  };

  if (isOwnProfile) {
    document.title = 'My profile - SkillBridge';
    els.backLabel.textContent = 'Back to browse';
    els.backLink.href = '/browse.html';
  } else {
    document.title = 'Profile - SkillBridge';
    els.backLabel.textContent = 'Back';
    els.backLink.href = '/browse.html';
  }

  loadProfile();

  function loadProfile() {
    SB.api('/api/users/' + fetchID)
      .then(function (data) {
        renderProfile(data);
      })
      .catch(function (err) {
        els.loading.hidden = true;
        if (err && err.status === 404) {
          els.notFound.hidden = false;
          return;
        }
        SB.toast(err && err.message ? err.message : 'Could not load profile.', { error: true });
      });
  }

  function renderProfile(data) {
    if (!data || !data.user) {
      els.loading.hidden = true;
      els.notFound.hidden = false;
      return;
    }

    const u = data.user;
    const role = u.role;
    const isFreelancer = role === 'FREELANCER';

    els.hAvatar.textContent = SB.initials(u.name);
    els.hName.textContent = u.name || 'Unknown';
    els.hRole.textContent = prettyRole(role);
    if (role === 'ADMIN') els.hRole.classList.add('admin');
    els.hJoined.textContent = 'Member since ' + formatMonthYear(u.createdAt);

    if (isFreelancer && (u.reviewCount || 0) > 0) {
      els.hRating.hidden = false;
      els.hRating.innerHTML =
        '<svg class="star" viewBox="0 0 24 24"><polygon points="12 2 15 9 22 9.5 17 14.5 18.5 22 12 18 5.5 22 7 14.5 2 9.5 9 9"/></svg>' +
        '<span>' + Number(u.avgRating || 0).toFixed(1) +
        ' (' + u.reviewCount + ' review' + (u.reviewCount === 1 ? '' : 's') + ')</span>';
    }

   if (isFreelancer) {
      els.flSec.hidden = false;
      renderPortfolio(u.portfolio);
      renderSkills(u.skills);
      renderGigs(data.gigs || []);
      renderReviews(data.reviews || []);
      loadEndorsementsReceived(fetchID);

      if (!isOwnProfile) {
        els.btnEndorse.hidden = false;
        els.btnEndorse.addEventListener('click', openEndorseModal);
      }
    }

    if (isOwnProfile) {
      setupAccountSection(u, isFreelancer);
    }

    els.loading.hidden = true;
    els.content.hidden = false;
  }

  function renderPortfolio(text) {
    if (text && String(text).trim()) {
      els.portfolioBody.textContent = text;
      els.portfolioBody.classList.remove('empty-text');
    } else {
      els.portfolioBody.textContent = isOwnProfile
        ? 'You have not added a portfolio yet. Add one in Account settings below.'
        : 'No portfolio yet.';
      els.portfolioBody.classList.add('empty-text');
    }
  }

  function renderSkills(text) {
    const trimmed = (text || '').trim();
    if (!trimmed) {
      els.skillsBody.innerHTML = '';
      const empty = document.createElement('div');
      empty.className = 'empty-text';
      empty.textContent = isOwnProfile
        ? 'You have not added skills yet. Add some in Account settings below.'
        : 'No skills listed.';
      els.skillsBody.appendChild(empty);
      return;
    }
    const chips = trimmed.split(',')
      .map(s => s.trim())
      .filter(s => s.length > 0);

    els.skillsBody.innerHTML = '';
    const wrap = document.createElement('div');
    wrap.className = 'skills-chips';
    chips.forEach(function (c) {
      const chip = document.createElement('span');
      chip.className = 'chip';
      chip.textContent = c;
      wrap.appendChild(chip);
    });
    els.skillsBody.appendChild(wrap);
  }

  function renderGigs(gigs) {
    els.gigsCount.textContent = '(' + gigs.length + ')';
    if (gigs.length === 0) {
      els.gigsGrid.innerHTML = '';
      els.gigsEmpty.hidden = false;
      if (isOwnProfile) {
        els.gigsEmptyTitle.textContent = 'No active gigs yet';
        els.gigsEmptyMsg.textContent = 'Create your first gig to start receiving orders.';
        els.gigsEmptyAction.innerHTML = '<a href="/gig-edit.html" class="btn btn-primary">Create your first gig</a>';
      } else {
        els.gigsEmptyTitle.textContent = 'No active gigs';
        els.gigsEmptyMsg.textContent = 'This freelancer has no gigs available right now.';
        els.gigsEmptyAction.innerHTML = '';
      }
      return;
    }
    els.gigsEmpty.hidden = true;

    let html = '';
    for (let i = 0; i < gigs.length; i++) {
      html += gigCardHtml(gigs[i], i);
    }
    els.gigsGrid.innerHTML = html;

    els.gigsGrid.querySelectorAll('[data-gigid]').forEach(function (el) {
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
        '<span>' + Number(g.freelancerAvgRating).toFixed(1) + '</span></div>'
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

  function renderReviews(reviews) {
    els.reviewsCount.textContent = '(' + reviews.length + ')';
    if (reviews.length === 0) {
      els.reviewsList.innerHTML = '';
      els.reviewsEmpty.hidden = false;
      return;
    }
    els.reviewsEmpty.hidden = true;

    let html = '';
    for (let i = 0; i < reviews.length; i++) {
      html += reviewCardHtml(reviews[i], i);
    }
    els.reviewsList.innerHTML = html;
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

  function starSvg() {
    return '<svg class="star" viewBox="0 0 24 24"><polygon points="12 2 15 9 22 9.5 17 14.5 18.5 22 12 18 5.5 22 7 14.5 2 9.5 9 9"/></svg>';
  }


  function loadEndorsementsReceived(targetID) {
    SB.api('/api/users/' + targetID + '/endorsements')
      .then(function (data) {
        var list = data.endorsements || [];
        els.endorseCount.textContent = '(' + list.length + ')';
        if (list.length === 0) {
          els.endorseEmpty.hidden = false;
          return;
        }
        var html = '';
        for (var i = 0; i < list.length; i++) {
          html += endorseRecvCardHtml(list[i], i);
        }
        els.endorseList.innerHTML = html;
      })
      .catch(function () {
        els.endorseCount.textContent = '(0)';
        els.endorseEmpty.hidden = false;
      });
  }

  function endorseRecvCardHtml(e, idx) {
    var stagger = 'animation-delay:' + (Math.min(idx, 10) * 0.04) + 's;';
    var weightStr = e.weight !== 1 ? ' (weight ' + e.weight.toFixed(1) + ')' : '';
    return [
      '<div class="glass-card endorse-recv-card" style="' + stagger + '">',
        '<div class="mini-avatar">' + SB.escapeHtml(SB.initials(e.fromUserName)) + '</div>',
        '<div class="e-info">',
          '<div class="e-who"><a href="/profile.html?id=' + e.fromUserID + '">' + SB.escapeHtml(e.fromUserName) + '</a>' + weightStr + '</div>',
          '<div class="e-when">' + SB.escapeHtml(formatDate(e.timestamp)) + '</div>',
        '</div>',
        '<span class="skill-chip">' + SB.escapeHtml(e.skill) + '</span>',
      '</div>'
    ].join('');
  }

  function openEndorseModal() {
    els.emSkill.value = '';
    els.emWeight.value = '1';
    els.emWeightVal.textContent = '1';
    hideBanner(els.emErrBanner);
    els.endorseModal.hidden = false;
    setTimeout(function () { try { els.emSkill.focus(); } catch (e) {} }, 50);
  }

  function closeEndorseModal() {
    els.endorseModal.hidden = true;
  }

  if (els.emCancel) {
    els.emCancel.addEventListener('click', closeEndorseModal);
  }
  if (els.endorseModal) {
    els.endorseModal.addEventListener('click', function (e) {
      if (e.target === els.endorseModal) closeEndorseModal();
    });
  }
  if (els.emWeight) {
    els.emWeight.addEventListener('input', function () {
      els.emWeightVal.textContent = els.emWeight.value;
    });
  }
  if (els.emConfirm) {
    els.emConfirm.addEventListener('click', function () {
      hideBanner(els.emErrBanner);
      var skill = els.emSkill.value.trim();
      if (!skill) {
        showBanner(els.emErrBanner, 'Skill is required.');
        return;
      }
      var weight = parseInt(els.emWeight.value, 10) || 1;

      els.emConfirm.disabled = true;
      els.emCancel.disabled = true;
      els.emConfirm.textContent = 'Endorsing...';

      SB.api('/api/endorsements', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          targetUserID: fetchID,
          skill: skill,
          weight: weight
        })
      })
        .then(function () {
          closeEndorseModal();
          SB.toast('Endorsement sent!');
          els.endorseList.innerHTML = '';
          els.endorseEmpty.hidden = true;
          loadEndorsementsReceived(fetchID);
        })
        .catch(function (err) {
          showBanner(els.emErrBanner, err && err.message ? err.message : 'Could not endorse.');
        })
        .then(function () {
          els.emConfirm.disabled = false;
          els.emCancel.disabled = false;
          els.emConfirm.textContent = 'Endorse';
        });
    });
  }


  function setupAccountSection(u, isFreelancer) {
    els.acSec.hidden = false;
    els.acName.value = u.name || '';

    var depositSec = document.getElementById('depositSection');
    if (u.role === 'CLIENT' && depositSec) {
      depositSec.hidden = false;
      document.getElementById('btnDeposit').addEventListener('click', onDeposit);
    }

    if (isFreelancer) {
      els.acPortfolioWrap.hidden = false;
      els.acSkillsWrap.hidden = false;
      els.acPortfolio.value = u.portfolio || '';
      els.acSkills.value = u.skills || '';
    }

    els.btnSaveProfile.addEventListener('click', onSaveProfile);
    els.btnChangePw.addEventListener('click', onChangePassword);
    els.btnDelete.addEventListener('click', openDeleteModal);

    els.dmCancel.addEventListener('click', closeDeleteModal);
    els.dmConfirm.addEventListener('click', onConfirmDelete);
    els.deleteModal.addEventListener('click', function (e) {
      if (e.target === els.deleteModal) closeDeleteModal();
    });
  }

  function onSaveProfile() {
    hideBanner(els.profileErrBanner);

    const newName = els.acName.value.trim();
    if (!newName) {
      showBanner(els.profileErrBanner, 'Name cannot be empty.');
      return;
    }
    if (newName.length > 100) {
      showBanner(els.profileErrBanner, 'Name cannot exceed 100 characters.');
      return;
    }

    const payload = { name: newName };

    if (!els.acPortfolioWrap.hidden) {
      payload.portfolio = els.acPortfolio.value;
    }
    if (!els.acSkillsWrap.hidden) {
      payload.skills = els.acSkills.value;
    }

    els.btnSaveProfile.disabled = true;
    els.btnSaveProfile.textContent = 'Saving...';

    SB.api('/api/me', {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    })
      .then(function (data) {
        SB.toast('Profile updated.');
        if (data && data.user) {
          SB.setSession(SB.getToken(), data.user);
        }
        if (data && data.user) {
          els.hName.textContent = data.user.name || els.hName.textContent;
          if (data.user.role === 'FREELANCER') {
            renderPortfolio(data.user.portfolio);
            renderSkills(data.user.skills);
          }
        }
      })
      .catch(function (err) {
        showBanner(els.profileErrBanner, err && err.message ? err.message : 'Could not save profile.');
      })
      .then(function () {
        els.btnSaveProfile.disabled = false;
        els.btnSaveProfile.textContent = 'Save changes';
      });
  }

  function onChangePassword() {
    hideBanner(els.pwErrBanner);

    const oldP = els.pwOld.value;
    const newP = els.pwNew.value;

    if (!oldP || !newP) {
      showBanner(els.pwErrBanner, 'Both fields are required.');
      return;
    }

    els.btnChangePw.disabled = true;
    els.btnChangePw.textContent = 'Changing...';

    SB.api('/api/me/password', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ oldPassword: oldP, newPassword: newP })
    })
      .then(function () {
        SB.toast('Password changed.');
        els.pwOld.value = '';
        els.pwNew.value = '';
      })
      .catch(function (err) {
        showBanner(els.pwErrBanner, err && err.message ? err.message : 'Could not change password.');
      })
      .then(function () {
        els.btnChangePw.disabled = false;
        els.btnChangePw.textContent = 'Change password';
      });
  }

  function onDeposit() {
    var banner = document.getElementById('depositErrBanner');
    hideBanner(banner);

    var input = document.getElementById('depositAmt');
    var amt = parseFloat(input.value);
    if (!amt || amt <= 0) {
      showBanner(banner, 'Enter a positive amount.');
      return;
    }
    if (amt > 1000000) {
      showBanner(banner, 'Maximum deposit is Rs 1,000,000.');
      return;
    }

    var btn = document.getElementById('btnDeposit');
    btn.disabled = true;
    btn.textContent = 'Adding...';

    SB.api('/api/me/deposit', {
      method: 'POST',
      body: { amount: amt }
    })
      .then(function (data) {
        SB.toast('Rs ' + SB.formatBalance(amt) + ' added to your balance.');
        input.value = '';
        if (data && typeof data.balance === 'number') {
          var u = SB.getUser();
          if (u) {
            u.balance = data.balance;
            SB.setSession(SB.getToken(), u);
          }
          var chips = document.querySelectorAll('.balance-chip');
          chips.forEach(function (c) {
            c.innerHTML = 'Balance &middot; <b>Rs ' + SB.formatBalance(data.balance) + '</b>';
          });
          var menuBal = document.querySelectorAll('.menu-balance');
          menuBal.forEach(function (c) {
            c.innerHTML = 'Balance &middot; <b>Rs ' + SB.formatBalance(data.balance) + '</b>';
          });
        }
      })
      .catch(function (err) {
        showBanner(banner, err && err.message ? err.message : 'Could not add funds.');
      })
      .then(function () {
        btn.disabled = false;
        btn.textContent = 'Add funds';
      });
  }

  function openDeleteModal() {
    els.dmPassword.value = '';
    hideBanner(els.dmErrBanner);
    els.deleteModal.hidden = false;
    setTimeout(function () { try { els.dmPassword.focus(); } catch (e) {} }, 50);
  }

  function closeDeleteModal() {
    els.deleteModal.hidden = true;
  }

  function onConfirmDelete() {
    hideBanner(els.dmErrBanner);
    const pwd = els.dmPassword.value;
    if (!pwd) {
      showBanner(els.dmErrBanner, 'Password is required.');
      return;
    }

    els.dmConfirm.disabled = true;
    els.dmCancel.disabled = true;
    els.dmConfirm.textContent = 'Deleting...';

    SB.api('/api/me', {
      method: 'DELETE',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ password: pwd })
    })
      .then(function () {
        SB.clearSession();
        SB.toast('Account deleted.');
        setTimeout(function () {
          window.location.replace('/');
        }, 700);
      })
      .catch(function (err) {
        els.dmConfirm.disabled = false;
        els.dmCancel.disabled = false;
        els.dmConfirm.textContent = 'Delete account';
        showBanner(els.dmErrBanner, err && err.message ? err.message : 'Could not delete account.');
      });
  }


  function showBanner(el, msg) {
    el.textContent = msg;
    el.classList.add('show');
  }
  function hideBanner(el) {
    el.textContent = '';
    el.classList.remove('show');
  }

  function prettyRole(r) {
    if (!r) return '';
    return r.charAt(0) + r.slice(1).toLowerCase();
  }

  function prettyCategory(c) {
    if (!c) return '';
    return c.charAt(0) + c.slice(1).toLowerCase();
  }

  function formatMonthYear(ts) {
    if (!ts) return '';
    const dPart = String(ts).split(' ')[0];
    const m = /^(\d{4})-(\d{2})/.exec(dPart);
    if (!m) return ts;
    const months = ['Jan','Feb','Mar','Apr','May','Jun',
                    'Jul','Aug','Sep','Oct','Nov','Dec'];
    const month = months[parseInt(m[2], 10) - 1] || '';
    return month + ' ' + m[1];
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

})();