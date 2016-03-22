document.onreadystatechange = function () {
  switch (document.readyState) {
  case 'interactive':
    break;
  case 'complete':
    new Player().start();
    break;
  }
};

Player = function Player() {
  this.el = {}
  this.el.body = document.querySelector('body');
  this.el.wrapper = document.querySelector('.wrapper');
};

Player.prototype.start = function () {
  this.stateChange(new Introduction(this));
};

Player.prototype.stateChange = function (state) {
  if (this.state) {
    this.state.exit();
  }

  this.state = state;
  this.el.body.className = state.stateClass;
  this.el.wrapper.className = "";

  state.entry();
};

Player.prototype.setStage = function (stageClass) {
  this.el.wrapper.className = stageClass;
};

Introduction = function(player) {
  this.player = player;
  this.stateClass = 'is-introduction';
};

Introduction.prototype.entry = function () {
  this.player.setStage('is-stage1');
  setTimeout(this.onFinishStage1.bind(this), 2000);
};

Introduction.prototype.onFinishStage1 = function () {
  this.player.setStage('is-stage2');
  setTimeout(this.onFinishStage2.bind(this), 2000);
};

Introduction.prototype.onFinishStage2 = function () {
  this.player.setStage('is-stage3');
  setTimeout(this.onFinishStage3.bind(this), 2000);
};

Introduction.prototype.onFinishStage3 = function () {
};
