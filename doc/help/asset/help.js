function updateTopicsLabel() {
  var body = document.querySelector('body');
  var topic = document.querySelector('a#topic');
  if (topic) {
    topic.textContent = body.classList.contains('is-menu-open') ? 'Hide topics' : 'Show topics';
  }
}

function toggleMenu(e) {
  var body = document.querySelector('body');
  body.classList.toggle('is-menu-open');
  if (body.classList.contains('is-menu-open')) {
    sessionStorage.setItem('is-menu-open', true);
  }
  else {
    sessionStorage.removeItem('is-menu-open');
  }
  updateTopicsLabel();

  if (e && e.preventDefault) {
    e.preventDefault();
  }
}

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
          var _anchor = unescape(lastPathComponent(element.nextSibling.href));
          sessionStorage.setItem(_anchor, true);
        }
      }

      list = list.parentNode.closest('ul');
    }
  }
}

function lastPathComponent(path) {
  var arr = path.split('/');
  return 0 < arr.length ? arr[arr.length - 1] : '';
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
    var topic = document.querySelector('a#topic');
    if (topic) {
      topic.addEventListener('click', toggleMenu);
    }

    var lastpath = lastPathComponent(location.pathname);
    if (0 < location.hash.length && lastpath.replace('.html', '') == location.hash.substring(1)) {
      location.hash = '';
    }

    if ('index.html' != lastpath) {
      if (!sessionStorage.getItem('session-started')) {
        toggleMenu();
      }
      var anchor = unescape(lastpath + location.hash);
      openTopic(anchor);
      selectTopic(anchor);
    }

    sessionStorage.setItem('session-started', true);

    if (sessionStorage.getItem('is-menu-open')) {
      body.classList.add('is-menu-open');
      updateTopicsLabel();
    }

    window.addEventListener('popstate', function () {
      var anchor = unescape(lastpath + location.hash);
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

if ("HelpViewer" in window && "showTOCButton" in window.HelpViewer) {
  var _enableTOC = function () {
    window.HelpViewer.showTOCButton(true, false, toggleMenu);
  };
  window.setTimeout(_enableTOC, 0);
  window.setTimeout(_enableTOC, 50);
  window.setTimeout(_enableTOC, 100);
  window.setTimeout(_enableTOC, 150);
  window.setTimeout(_enableTOC, 200);
  window.setTimeout(_enableTOC, 250);
}
