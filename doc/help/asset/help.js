function openTopic(anchor) {
  var link = document.querySelector('nav a[href="'+anchor+'"]');

  if (link) {
    var list = link.parentNode.closest('ul');

    while (list) {
      if (list && list.previousSibling && list.previousSibling.previousSibling) {
        var element = list.previousSibling.previousSibling;
        if ('A' == element.nodeName) {
          element = element.previousSibling;
        }
        if (element && 'INPUT' == element.nodeName && 'checkbox' == element.type) {
          element.checked = true;
          var href = element.nextSibling.href;
          var _anchor = unescape(lastPathComponent(href));
          sessionStorage.setItem(_anchor, true);
        }
      }

      list = list.parentNode.closest('ul');
    }
  }
}

function lastPathComponent(path) {
  var arr = path.split('#');
  var path = arr[0];
  var hash = 1 < arr.length ? '#'+arr[1] : '';
  var arr = path.split('/');
  return (0 < arr.length ? arr[arr.length - 1] : '') + hash;
}

function selectTopic(anchor) {
  var links = document.querySelectorAll('nav a');
  for (var i = 0; i < links.length; ++i) {
    var link = links[i];
    if (lastPathComponent(link.href) == anchor) {
      link.closest('li').classList.add('is-active');
    }
    else {
      link.closest('li').classList.remove('is-active');
    }
  }
}

document.onreadystatechange = function () {
  switch (document.readyState) {
  case 'interactive':
    var body = document.querySelector('body');

    var nav = document.querySelector('nav');
    nav.innerHTML = window.__tocHTML;
    nav.onscroll = function () {
      sessionStorage.setItem('nav:scrollTop', nav.scrollTop);
    };
    setTimeout(function () {
      var scrollTop = sessionStorage.getItem('nav:scrollTop') || 0;
      if (0 < scrollTop) {
        nav.scrollTop = scrollTop;
      }
    }, 0);

    var lastpath = lastPathComponent(location.pathname);
    if (0 < location.hash.length && lastpath.replace('.html', '') == location.hash.substring(1)) {
      location.hash = '';
    }

    if ('index.html' != lastpath) {
      var anchor = unescape(lastpath + location.hash);
      openTopic(anchor);
      selectTopic(anchor);
    }

    window.addEventListener('popstate', function () {
      var anchor = unescape(lastPathComponent(location.href));
      openTopic(anchor);
      selectTopic(anchor);
    });

    var checkboxes = document.querySelectorAll('nav input[type="checkbox"]');
    for (var i = 0; i < checkboxes.length; ++i) {
      var checkbox = checkboxes[i];
      var anchor = unescape(lastPathComponent(checkbox.nextSibling.href));
      if (sessionStorage.getItem(anchor)) {
        checkbox.checked = true;
      }

      checkbox.addEventListener('click', function () {
        var anchor = unescape(lastPathComponent(this.nextSibling.href));
        if (this.checked) {
          sessionStorage.setItem(anchor, true);
        }
        else {
          sessionStorage.removeItem(anchor);
        }
      });
    }

    break;

  case 'complete':
    break;
  }
};
