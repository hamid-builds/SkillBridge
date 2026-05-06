(function () 
{
    'use strict';

    SB.requireAuth();
    SB.renderTopNav({ activeLink: 'admin' });

    var user = SB.getUser();
    if (!user || user.role !== 'ADMIN') {
        window.location.href = '/browse.html';
        return;
    }

    var $tabs       = document.getElementById('adminTabs');
    var $secHead    = document.getElementById('secHead');
    var $secTitle   = document.getElementById('sectionTitle');
    var $itemCount  = document.getElementById('itemCount');
    var $empty      = document.getElementById('emptyState');
    var $emptyTitle = document.getElementById('emptyTitle');
    var $emptyMsg   = document.getElementById('emptyMsg');

    var panels = {
        users:   document.getElementById('panelUsers'),
        gigs:    document.getElementById('panelGigs'),
        orders:  document.getElementById('panelOrders'),
        reviews: document.getElementById('panelReviews')
    };

    var $modal       = document.getElementById('confirmModal');
    var $modalTitle   = document.getElementById('confirmTitle');
    var $modalBody    = document.getElementById('confirmBody');
    var $modalCancel  = document.getElementById('confirmCancelBtn');
    var $modalOk      = document.getElementById('confirmOkBtn');
    var $modalClose   = document.getElementById('confirmClose');
    var confirmCb     = null;

    function showConfirm(title, body, okLabel, cb) {
        $modalTitle.textContent = title;
        $modalBody.textContent = body;
        $modalOk.textContent = okLabel || 'Delete';
        confirmCb = cb;
        $modal.classList.add('show');
    }
    function hideConfirm() {
        $modal.classList.remove('show');
        confirmCb = null;
    }
    $modalCancel.onclick = hideConfirm;
    $modalClose.onclick  = hideConfirm;
    $modal.addEventListener('click', function (e) {
        if (e.target === $modal) hideConfirm();
    });
    $modalOk.onclick = function () {
        if (confirmCb) confirmCb();
        hideConfirm();
    };

    var activeTab = 'users';
    var loaded = { users: false, gigs: false, orders: false, reviews: false };

    $tabs.addEventListener('click', function (e) {
        var btn = e.target.closest('button');
        if (!btn || !btn.dataset.tab) return;
        switchTab(btn.dataset.tab);
    });

    function switchTab(tab) {
        activeTab = tab;
        var btns = $tabs.querySelectorAll('button');
        for (var i = 0; i < btns.length; i++) {
            btns[i].classList.toggle('on', btns[i].dataset.tab === tab);
        }
        for (var key in panels) {
            panels[key].hidden = (key !== tab);
        }
        $empty.hidden = true;
        $secHead.style.display = 'none';

        if (!loaded[tab]) {
            loaded[tab] = true;
            loaders[tab]();
        }
    }

    function esc(s) { return SB.escapeHtml(s || ''); }

    function skelRows(panel, cols, n) {
        var html = '<div class="table-wrap"><table class="admin-table"><thead><tr>';
        for (var c = 0; c < cols; c++) html += '<th>&nbsp;</th>';
        html += '</tr></thead><tbody>';
        for (var r = 0; r < n; r++) {
            html += '<tr class="skel-row">';
            for (var c2 = 0; c2 < cols; c2++) {
                var w = 40 + Math.random() * 50;
                html += '<td><div class="skel-bar" style="width:' + w + '%"></div></td>';
            }
            html += '</tr>';
        }
        html += '</tbody></table></div>';
        panel.innerHTML = html;
    }

    function showSection(title, count) {
        $secHead.style.display = '';
        $secTitle.innerHTML = esc(title) + ' <span class="count">' + count + '</span>';
    }

    function showEmpty(title, msg) {
        $emptyTitle.textContent = title;
        $emptyMsg.textContent = msg;
        $empty.hidden = false;
    }

    function roleBadge(role) {
        var cls = (role || '').toLowerCase();
        return '<span class="role-badge ' + cls + '">' + esc(role) + '</span>';
    }

    function activePill(active) {
        if (active) return '<span class="active-pill yes"><span class="dot"></span>Active</span>';
        return '<span class="active-pill no"><span class="dot"></span>Inactive</span>';
    }

    function statusBadge(status) {
        var cls = (status || '').toLowerCase();
        return '<span class="status-badge ' + cls + '"><span class="status-dot"></span>' + esc(status) + '</span>';
    }

    function starsHtml(rating) {
        var html = '<span class="star-display">';
        for (var i = 1; i <= 5; i++) {
            var cls = i <= rating ? 'on' : 'off';
            html += '<svg class="' + cls + '" viewBox="0 0 24 24"><polygon points="12 2 15 9 22 9.5 17 14.5 18.5 22 12 18 5.5 22 7 14.5 2 9.5 9 9"/></svg>';
        }
        html += '</span>';
        return html;
    }

    function formatDate(d) {
        if (!d) return '';
        return d.substring(0, 10);
    }

    var loaders = {};

    loaders.users = function () {
        skelRows(panels.users, 6, 5);
        SB.api('/api/admin/users').then(function (data) {
            var users = data.users || [];
            showSection('Users', users.length);
            if (users.length === 0) {
                panels.users.innerHTML = '';
                showEmpty('No users', 'The platform has no registered users.');
                return;
            }
            renderUsersTable(users);
        }).catch(function (err) {
            panels.users.innerHTML = '';
            SB.toast(err.message || 'Failed to load users', { error: true });
        });
    };

    function renderUsersTable(users) {
        var html = '<div class="table-wrap"><table class="admin-table">';
        html += '<thead><tr>'
            + '<th>ID</th><th>Name</th><th>Email</th>'
            + '<th>Role</th><th>Balance</th><th>Joined</th><th></th>'
            + '</tr></thead><tbody>';
        for (var i = 0; i < users.length; i++) {
            var u = users[i];
            var delay = (i * 0.04) + 's';
            html += '<tr style="animation-delay:' + delay + '">'
                + '<td class="cell-mono">' + u.userID + '</td>'
                + '<td>' + esc(u.name) + '</td>'
                + '<td style="color:var(--text-dim)">' + esc(u.email) + '</td>'
                + '<td>' + roleBadge(u.role) + '</td>'
                + '<td class="cell-mono">' + SB.formatBalance(u.balance) + '</td>'
                + '<td style="color:var(--text-dim)">' + formatDate(u.createdAt) + '</td>'
                + '<td>';
            if (u.role !== 'ADMIN') {
                html += '<button class="btn-table-danger" data-delete-user="' + u.userID + '" data-name="' + esc(u.name) + '">Delete</button>';
            }
            html += '</td></tr>';
        }
        html += '</tbody></table></div>';
        panels.users.innerHTML = html;

        panels.users.addEventListener('click', function (e) {
            var btn = e.target.closest('[data-delete-user]');
            if (!btn) return;
            var uid = parseInt(btn.dataset.deleteUser, 10);
            var name = btn.dataset.name;
            showConfirm(
                'Delete User',
                'Permanently delete "' + name + '" and all their gigs, orders, messages, reviews, and endorsements? This cannot be undone.',
                'Delete User',
                function () { deleteUser(uid); }
            );
        });
    }

    function deleteUser(uid) {
        SB.api('/api/admin/users/' + uid, { method: 'DELETE' }).then(function () {
            SB.toast('User deleted');
            loaded.users = false;
            loaders.users();
            loaded.gigs = false;
            loaded.orders = false;
            loaded.reviews = false;
        }).catch(function (err) {
            SB.toast(err.message || 'Failed to delete user', { error: true });
        });
    }

    loaders.gigs = function () {
        skelRows(panels.gigs, 7, 5);
        SB.api('/api/admin/gigs').then(function (data) {
            var gigs = data.gigs || [];
            showSection('Gigs', gigs.length);
            if (gigs.length === 0) {
                panels.gigs.innerHTML = '';
                showEmpty('No gigs', 'No gigs have been created on the platform.');
                return;
            }
            renderGigsTable(gigs);
        }).catch(function (err) {
            panels.gigs.innerHTML = '';
            SB.toast(err.message || 'Failed to load gigs', { error: true });
        });
    };

    function renderGigsTable(gigs) {
        var html = '<div class="table-wrap"><table class="admin-table">';
        html += '<thead><tr>'
            + '<th>ID</th><th>Title</th><th>Owner</th>'
            + '<th>Price</th><th>Category</th><th>Status</th><th>Created</th><th></th>'
            + '</tr></thead><tbody>';
        for (var i = 0; i < gigs.length; i++) {
            var g = gigs[i];
            var delay = (i * 0.04) + 's';
            html += '<tr style="animation-delay:' + delay + '">'
                + '<td class="cell-mono">' + g.gigID + '</td>'
                + '<td class="cell-title"><a href="/gig.html?id=' + g.gigID + '" class="btn-table-link" style="padding:0">' + esc(g.title) + '</a></td>'
                + '<td><a href="/profile.html?id=' + g.ownerID + '" class="btn-table-link" style="padding:0">' + esc(g.ownerName) + '</a></td>'
                + '<td class="cell-mono">' + SB.formatPrice(g.price) + '</td>'
                + '<td style="color:var(--text-dim)">' + esc(g.category) + '</td>'
                + '<td>' + activePill(g.isActive) + '</td>'
                + '<td style="color:var(--text-dim)">' + formatDate(g.createdAt) + '</td>'
                + '<td><button class="btn-table-danger" data-delete-gig="' + g.gigID + '" data-title="' + esc(g.title) + '">Delete</button></td>'
                + '</tr>';
        }
        html += '</tbody></table></div>';
        panels.gigs.innerHTML = html;

        panels.gigs.addEventListener('click', function (e) {
            var btn = e.target.closest('[data-delete-gig]');
            if (!btn) return;
            var gid = parseInt(btn.dataset.deleteGig, 10);
            var title = btn.dataset.title;
            showConfirm(
                'Delete Gig',
                'Permanently delete "' + title + '"? This cannot be undone.',
                'Delete Gig',
                function () { deleteGig(gid); }
            );
        });
    }

    function deleteGig(gid) {
        SB.api('/api/gigs/' + gid, { method: 'DELETE' }).then(function () {
            SB.toast('Gig deleted');
            loaded.gigs = false;
            loaders.gigs();
        }).catch(function (err) {
            SB.toast(err.message || 'Failed to delete gig', { error: true });
        });
    }

    loaders.orders = function () {
        skelRows(panels.orders, 7, 5);
        SB.api('/api/orders?role=all').then(function (data) {
            var orders = data.orders || [];
            showSection('Orders', orders.length);
            if (orders.length === 0) {
                panels.orders.innerHTML = '';
                showEmpty('No orders', 'No orders have been placed on the platform.');
                return;
            }
            renderOrdersTable(orders);
        }).catch(function (err) {
            panels.orders.innerHTML = '';
            SB.toast(err.message || 'Failed to load orders', { error: true });
        });
    };

    function renderOrdersTable(orders) {
        var html = '<div class="table-wrap"><table class="admin-table">';
        html += '<thead><tr>'
            + '<th>ID</th><th>Gig</th><th>Seller</th>'
            + '<th>Amount</th><th>Status</th><th>Placed</th><th>Deadline</th>'
            + '</tr></thead><tbody>';
        for (var i = 0; i < orders.length; i++) {
            var o = orders[i];
            var delay = (i * 0.04) + 's';
            html += '<tr style="animation-delay:' + delay + '">'
                + '<td class="cell-mono">' + o.orderID + '</td>'
                + '<td class="cell-title">' + esc(o.gigTitle) + '</td>'
                + '<td><a href="/profile.html?id=' + o.otherPartyID + '" class="btn-table-link" style="padding:0">' + esc(o.otherPartyName) + '</a></td>'
                + '<td class="cell-mono">' + SB.formatPrice(o.amount) + '</td>'
                + '<td>' + statusBadge(o.status) + '</td>'
                + '<td style="color:var(--text-dim)">' + formatDate(o.placedAt) + '</td>'
                + '<td style="color:var(--text-dim)">' + formatDate(o.deadline) + '</td>'
                + '</tr>';
        }
        html += '</tbody></table></div>';
        panels.orders.innerHTML = html;
    }

    var reviewFreelancers = [];

    loaders.reviews = function () {
        panels.reviews.innerHTML = '<div style="color:var(--text-dim);font-size:13px;">Loading freelancers...</div>';
        SB.api('/api/admin/users').then(function (data) {
            var users = data.users || [];
            reviewFreelancers = users.filter(function (u) { return u.role === 'FREELANCER'; });

            if (reviewFreelancers.length === 0) {
                panels.reviews.innerHTML = '';
                showSection('Reviews', 0);
                showEmpty('No freelancers', 'There are no freelancers to show reviews for.');
                return;
            }

            var html = '<div class="review-filter">'
                + '<label for="freelancerSelect">Freelancer:</label>'
                + '<select id="freelancerSelect">'
                + '<option value="">Select a freelancer</option>';
            for (var i = 0; i < reviewFreelancers.length; i++) {
                var f = reviewFreelancers[i];
                html += '<option value="' + f.userID + '">' + esc(f.name) + '</option>';
            }
            html += '</select></div>';
            html += '<div id="reviewTableArea"></div>';
            panels.reviews.innerHTML = html;

            document.getElementById('freelancerSelect').addEventListener('change', function () {
                var fid = parseInt(this.value, 10);
                if (!fid) {
                    document.getElementById('reviewTableArea').innerHTML = '';
                    $secHead.style.display = 'none';
                    return;
                }
                loadReviewsFor(fid);
            });
        }).catch(function (err) {
            panels.reviews.innerHTML = '';
            SB.toast(err.message || 'Failed to load users', { error: true });
        });
    };

    function loadReviewsFor(freelancerID) {
        var area = document.getElementById('reviewTableArea');
        area.innerHTML = '<div style="padding:20px 0;color:var(--text-dim);font-size:13px;">Loading reviews...</div>';

        SB.api('/api/users/' + freelancerID + '/reviews?limit=50').then(function (data) {
            var reviews = data.reviews || [];
            showSection('Reviews', reviews.length);

            if (reviews.length === 0) {
                area.innerHTML = '<div style="padding:20px 0;color:var(--text-dim);font-size:13px;">No reviews for this freelancer.</div>';
                return;
            }

            var html = '<div class="table-wrap"><table class="admin-table">';
            html += '<thead><tr>'
                + '<th>ID</th><th>Reviewer</th><th>Rating</th>'
                + '<th>Comment</th><th>Date</th><th></th>'
                + '</tr></thead><tbody>';
            for (var i = 0; i < reviews.length; i++) {
                var r = reviews[i];
                var delay = (i * 0.04) + 's';
                html += '<tr style="animation-delay:' + delay + '">'
                    + '<td class="cell-mono">' + r.reviewID + '</td>'
                    + '<td>' + esc(r.reviewerName) + '</td>'
                    + '<td>' + starsHtml(r.rating) + '</td>'
                    + '<td class="review-comment" title="' + esc(r.comment) + '">' + esc(r.comment) + '</td>'
                    + '<td style="color:var(--text-dim)">' + formatDate(r.createdAt) + '</td>'
                    + '<td><button class="btn-table-danger" data-delete-review="' + r.reviewID + '">Delete</button></td>'
                    + '</tr>';
            }
            html += '</tbody></table></div>';
            area.innerHTML = html;

            area.addEventListener('click', function (e) {
                var btn = e.target.closest('[data-delete-review]');
                if (!btn) return;
                var rid = parseInt(btn.dataset.deleteReview, 10);
                showConfirm(
                    'Delete Review',
                    'Permanently delete this review? This cannot be undone.',
                    'Delete Review',
                    function () { deleteReview(rid, freelancerID); }
                );
            });
        }).catch(function (err) {
            area.innerHTML = '';
            SB.toast(err.message || 'Failed to load reviews', { error: true });
        });
    }

    function deleteReview(rid, freelancerID) {
        SB.api('/api/admin/reviews/' + rid, { method: 'DELETE' }).then(function () {
            SB.toast('Review deleted');
            loadReviewsFor(freelancerID);
        }).catch(function (err) {
            SB.toast(err.message || 'Failed to delete review', { error: true });
        });
    }

    switchTab('users');

})();