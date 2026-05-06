(function () {
  'use strict';

  if (!SB.requireAuth()) return;

  const user = SB.getUser();

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

  const editGigID = readGigIdFromUrl();
  const isEditMode = editGigID !== null;

  SB.renderTopNav({ activeLink: isEditMode ? 'browse' : 'mygigs' });

  const $loading = document.getElementById('loadingState');
  const $form = document.getElementById('formState');
  const $forbidden = document.getElementById('forbiddenState');
  const $forbiddenTitle = document.getElementById('forbiddenTitle');
  const $forbiddenMsg = document.getElementById('forbiddenMsg');

  const $modeEyebrow = document.getElementById('modeEyebrow');
  const $pageTitle = document.getElementById('pageTitle');
  const $pageSubtitle = document.getElementById('pageSubtitle');
  const $backLabel = document.getElementById('backLabel');
  const $backLink = document.getElementById('backLink');
  const docTitle = document;

  const $fldCategory = document.getElementById('fldCategory');
  const $fldTitle = document.getElementById('fldTitle');
  const $fldDesc = document.getElementById('fldDesc');
  const $fldPrice = document.getElementById('fldPrice');

  const $errCategory = document.getElementById('errCategory');
  const $errTitle = document.getElementById('errTitle');
  const $errDesc = document.getElementById('errDesc');
  const $errPrice = document.getElementById('errPrice');
  const $errBanner = document.getElementById('errBanner');

  const $cntTitle = document.getElementById('cntTitle');
  const $cntDesc = document.getElementById('cntDesc');

  const $btnSubmit = document.getElementById('btnSubmit');
  const $btnCancel = document.getElementById('btnCancel');

  if (isEditMode) {
    document.title = 'Edit gig - SkillBridge';
    $modeEyebrow.textContent = 'Edit gig';
    $pageTitle.textContent = 'Edit your gig';
    $pageSubtitle.textContent = 'Update the details. Changes take effect immediately and your gig stays visible during the edit.';
    $btnSubmit.textContent = 'Save changes';
    $backLink.href = '/gig.html?id=' + editGigID;
    $backLabel.textContent = 'Back to gig';
  } else {
    document.title = 'New gig - SkillBridge';
    $modeEyebrow.textContent = 'New gig';
    $pageTitle.textContent = 'Create a new gig';
    $pageSubtitle.textContent = 'Set a clear title, describe what you offer, and pick a fair price. Clients will see this on browse.';
    $btnSubmit.textContent = 'Publish gig';
    $backLink.href = '/browse.html';
    $backLabel.textContent = 'Back to gigs';
  }

  if (isEditMode) {
    bootstrapEditMode(editGigID);
  } else {
    if (user.role !== 'FREELANCER') {
      $form.hidden = true;
      $forbiddenTitle.textContent = 'Only freelancers can create gigs';
      $forbiddenMsg.textContent = 'Switch to a freelancer account to post a gig on SkillBridge.';
      $forbidden.hidden = false;
      return;
    }
    wireForm();
  }


  function bootstrapEditMode(id) {
    $form.hidden = true;
    $loading.hidden = false;

    SB.api('/api/gigs/' + id)
      .then(function (data) {
        if (!data || !data.gig || !data.viewer) {
          throw new Error('Malformed gig response.');
        }

        if (!data.viewer.canEdit) {
          SB.toast('You can\'t edit this gig.', { error: true });
          setTimeout(function () {
            window.location.replace('/gig.html?id=' + id);
          }, 600);
          return;
        }

        $fldCategory.value = data.gig.category || '';
        $fldTitle.value = data.gig.title || '';
        $fldDesc.value = data.gig.description || '';
        const p = data.gig.price;
        $fldPrice.value = (typeof p === 'number') ? String(p) : '';

        updateCounters();

        $loading.hidden = true;
        $form.hidden = false;
        wireForm();
      })
      .catch(function (err) {
        $loading.hidden = true;
        if (err && err.status === 404) {
          $forbiddenTitle.textContent = 'Gig not found';
          $forbiddenMsg.textContent = 'This gig does not exist or has been removed.';
          $forbidden.hidden = false;
          return;
        }
        $form.hidden = false;
        wireForm();
        showBanner(err && err.message ? err.message : 'Could not load this gig.');
      });
  }


  function wireForm() {
    $fldTitle.addEventListener('input', updateCounters);
    $fldDesc.addEventListener('input', updateCounters);

    $fldCategory.addEventListener('change', function () { clearFieldError('Category'); });
    $fldTitle.addEventListener('input', function () { clearFieldError('Title'); });
    $fldDesc.addEventListener('input', function () { clearFieldError('Desc'); });
    $fldPrice.addEventListener('input', function () { clearFieldError('Price'); });

    $btnSubmit.addEventListener('click', onSubmit);
    $btnCancel.addEventListener('click', onCancel);

    $fldTitle.addEventListener('keydown', function (e) {
      if (e.key === 'Enter') {
        e.preventDefault();
        onSubmit();
      }
    });
    $fldPrice.addEventListener('keydown', function (e) {
      if (e.key === 'Enter') {
        e.preventDefault();
        onSubmit();
      }
    });

    updateCounters();
  }


  function updateCounters() {
    setCounter($cntTitle, $fldTitle.value.length, 100, 3);
    setCounter($cntDesc, $fldDesc.value.length, 2000, 10);
  }

  function setCounter(el, current, max, min) {
    el.textContent = current + ' / ' + max;
    el.classList.remove('warn', 'over');
    if (current > max) {
      el.classList.add('over');
      return;
    }
    if (current >= Math.floor(max * 0.9)) {
      el.classList.add('warn');
    }
    
    void min; 
  }


  function showFieldError(name, msg) {
    const errEl = document.getElementById('err' + name);
    const fldEl = document.getElementById('fld' + name);
    if (!errEl || !fldEl) return;
    errEl.textContent = msg;
    errEl.classList.add('show');
    fldEl.classList.add('has-error');
  }

  function clearFieldError(name) {
    const errEl = document.getElementById('err' + name);
    const fldEl = document.getElementById('fld' + name);
    if (errEl) {
      errEl.textContent = '';
      errEl.classList.remove('show');
    }
    if (fldEl) fldEl.classList.remove('has-error');
  }

  function clearAllErrors() {
    ['Category', 'Title', 'Desc', 'Price'].forEach(clearFieldError);
    hideBanner();
  }

  function showBanner(msg) {
    $errBanner.textContent = msg;
    $errBanner.classList.add('show');
  }

  function hideBanner() {
    $errBanner.textContent = '';
    $errBanner.classList.remove('show');
  }


  function onCancel() {
    if (isEditMode) {
      window.location.href = '/gig.html?id=' + editGigID;
    } else {
      window.location.href = '/browse.html';
    }
  }

  function onSubmit() {
    clearAllErrors();

    const category = $fldCategory.value;
    const title = $fldTitle.value.trim();
    const desc = $fldDesc.value.trim();
    const priceStr = $fldPrice.value.trim();
    const price = parseFloat(priceStr);

    let firstBad = null;

    if (!category) {
      showFieldError('Category', 'Pick a category.');
      firstBad = firstBad || $fldCategory;
    }
    if (!title) {
      showFieldError('Title', 'Title is required.');
      firstBad = firstBad || $fldTitle;
    } else if (title.length < 3) {
      showFieldError('Title', 'Title must be at least 3 characters.');
      firstBad = firstBad || $fldTitle;
    } else if (title.length > 100) {
      showFieldError('Title', 'Title cannot exceed 100 characters.');
      firstBad = firstBad || $fldTitle;
    }
    if (!desc) {
      showFieldError('Desc', 'Description is required.');
      firstBad = firstBad || $fldDesc;
    } else if (desc.length < 10) {
      showFieldError('Desc', 'Description must be at least 10 characters.');
      firstBad = firstBad || $fldDesc;
    } else if (desc.length > 2000) {
      showFieldError('Desc', 'Description cannot exceed 2000 characters.');
      firstBad = firstBad || $fldDesc;
    }
    if (!priceStr) {
      showFieldError('Price', 'Price is required.');
      firstBad = firstBad || $fldPrice;
    } else if (!isFinite(price) || price <= 0) {
      showFieldError('Price', 'Price must be a positive number.');
      firstBad = firstBad || $fldPrice;
    } else if (price >= 1000000) {
      showFieldError('Price', 'Price must be less than 1,000,000.');
      firstBad = firstBad || $fldPrice;
    }

    if (firstBad) {
      try { firstBad.focus(); } catch (e) {}
      return;
    }

    const payload = {
      title: title,
      description: desc,
      price: price,
      category: category
    };

    $btnSubmit.disabled = true;
    $btnCancel.disabled = true;
    const originalText = $btnSubmit.textContent;
    $btnSubmit.textContent = isEditMode ? 'Saving...' : 'Publishing...';

    const path = isEditMode ? '/api/gigs/' + editGigID : '/api/gigs';
    const method = isEditMode ? 'PUT' : 'POST';

    SB.api(path, {
      method: method,
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    })
      .then(function (data) {
        if (!data || !data.gig || typeof data.gig.gigID !== 'number') {
          throw new Error('Malformed response from server.');
        }
        SB.toast(isEditMode ? 'Gig updated.' : 'Gig published.');
        setTimeout(function () {
          window.location.href = '/gig.html?id=' + data.gig.gigID;
        }, 600);
      })
      .catch(function (err) {
        $btnSubmit.disabled = false;
        $btnCancel.disabled = false;
        $btnSubmit.textContent = originalText;
        const msg = (err && err.message) ? err.message : 'Submission failed.';
        applyServerError(msg);
      });
  }

  function applyServerError(msg) {
    const lower = String(msg).toLowerCase();

    if (lower.indexOf('title') !== -1) {
      showFieldError('Title', msg);
      try { $fldTitle.focus(); } catch (e) {}
      return;
    }
    if (lower.indexOf('description') !== -1) {
      showFieldError('Desc', msg);
      try { $fldDesc.focus(); } catch (e) {}
      return;
    }
    if (lower.indexOf('price') !== -1) {
      showFieldError('Price', msg);
      try { $fldPrice.focus(); } catch (e) {}
      return;
    }
    if (lower.indexOf('category') !== -1) {
      showFieldError('Category', msg);
      try { $fldCategory.focus(); } catch (e) {}
      return;
    }
    showBanner(msg);
  }

})();