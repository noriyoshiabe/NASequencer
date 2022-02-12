(function() {
  var CLIENT_ID_KEY = "GA_CLIENT_ID_ONLY_FOR_THIS_SITE";

  window.dataLayer = window.dataLayer || [];
  function gtag(){dataLayer.push(arguments);}
  gtag('js', new Date());

  if (window.localStorage) {
    var clientId = window.localStorage.getItem(CLIENT_ID_KEY);
    if (!clientId) {
      clientId = Array.apply(null, Array(32)).map(() => Math.floor(Math.random() * 16).toString(16)).join('');
      window.localStorage.setItem(CLIENT_ID_KEY, clientId);
    }

    gtag('config', 'UA-77464123-X', {client_storage: 'none', client_id: clientId});
  }

  document.onreadystatechange = function () {
    if ('complete' == document.readyState) {
      var appStore = document.querySelector('a#app-store');
      if (appStore) {
        appStore.addEventListener('click', function () {
          gtag('event', 'click', {event_category: 'AppStore'});
        });
      }
    }
  };
})();
