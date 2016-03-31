function updateTopicsLabel() {
  var body = document.querySelector('body');
  var topic = document.querySelector('a#topic');
  topic.textContent = body.classList.contains('is-menu-open') ? 'Hide topics' : 'Show topics';
}

function toggleMenu(e) {
  document.querySelector('body').classList.toggle('is-menu-open');
  updateTopicsLabel();
  e.preventDefault();
}

document.onreadystatechange = function () {
  switch (document.readyState) {
  case 'interactive':
    var body = document.querySelector('body');
    var topic = document.querySelector('a#topic');
    topic.addEventListener('click', toggleMenu);

    var transits = document.querySelectorAll('.js-transit');
    for (var i = 0; i < transits.length; ++i) {
      transits[i].addEventListener('click', function () {
        history.pushState({}, document.title, this.getAttribute('href'));

        var isMenuOpen = body.classList.contains('is-menu-open');
        body.className = this.getAttribute('href').substring(1).split('__')[0];
        if (isMenuOpen) {
          body.classList.add('is-menu-open');
        }

        setTimeout(function () {
          if (!location.hash.match(/__/)) {
            body.scrollTop = 0;
          }
        }, 0);
      });
    }

    if (!location.hash) {
      location.hash = '#top';
    }
    body.className = location.hash.substring(1).split('__')[0];

    if ('#top' != location.hash) {
      body.classList.add('is-menu-open');
      updateTopicsLabel();
    }

    window.addEventListener('popstate', function () {
      var isMenuOpen = body.classList.contains('is-menu-open');
      body.className = location.hash.substring(1).split('__')[0];
      if (isMenuOpen) {
        body.classList.add('is-menu-open');
      }

      body.style.overflow = 'hidden';
      setTimeout(function () {
        body.style.overflow = '';
      }, 10);
    });

    break;
  case 'complete':
    break;
  }
};

if ("HelpViewer" in window && "showTOCButton" in window.HelpViewer) {
	window.setTimeout(function () {
		window.HelpViewer.showTOCButton(true, toggleMenu, toggleMenu);
		window.HelpViewer.setTOCButton(true);
	}, 100);
}
