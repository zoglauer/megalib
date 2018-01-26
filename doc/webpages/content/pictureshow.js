/* Mechanics of the picture slide show */

var ImageCurrent = 0;
var ImageIntervalTime = 10000;
var ImageInterval = setInterval("ImageSwitchForward()", ImageIntervalTime);
var ImageShuffled = [];

function ImageShow() {
  var Images = document.getElementsByClassName("nd-ps-figure");

  if (ImageCurrent === 0 || ImageShuffled.length != Images.length) {
    ImageShuffled = [];
    for (var i = 0; i < Images.length; ++i) {
      ImageShuffled.push(i);
    }
    
    for (var i = ImageShuffled.length - 1; i > 0; i--) {
      var j = Math.floor(Math.random() * (i + 1));
      var temp = ImageShuffled[i];
      ImageShuffled[i] = ImageShuffled[j];
      ImageShuffled[j] = temp;
    }    
  }
  
  for (var i = 0; i < Images.length; i++) {
    Images[i].classList.remove("nd-ps-show-figure");
  }
  Images[ImageShuffled[ImageCurrent]].classList.add("nd-ps-show-figure");
  clearInterval(ImageInterval);
  ImageInterval = setInterval("ImageSwitchForward()", ImageIntervalTime);
}

function ImageSwitchForward() {
  //document.getElementById("demo").innerHTML = "Forward  ";
  var Images = document.getElementsByClassName("nd-ps-figure");
  ImageCurrent++;
  //document.getElementById("demo").innerHTML += ImageCurrent;
  if (ImageCurrent >= Images.length) {
    ImageCurrent = 0;
  }
  ImageShow();
}

function ImageSwitchBackward() {
  var Images = document.getElementsByClassName("nd-ps-figure");
  ImageCurrent--;
  if (ImageCurrent < 0) {
    ImageCurrent = Images.length - 1;
  }
  ImageShow();
}
window.onload = ImageShow;

