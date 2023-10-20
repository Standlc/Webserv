document.getElementById("animateButton").addEventListener("click", () => {
  const body = document.querySelector("body");

  let randRgbComponents = [randChar(), randChar(), randChar()];
  body.style.backgroundColor = `rgb(${randRgbComponents[0]}, ${randRgbComponents[1]}, ${randRgbComponents[2]})`;
});

const randChar = () => {
  return Math.floor(Math.random() * 255);
}

const getOppositeColor = (randRgbComponents) => {
  return `rgb(${255 - randRgbComponents[0]}, ${255 - randRgbComponents[1]}, ${255 - randRgbComponents[2]})`;
}

window.onload = () => {
  const body = document.querySelector("body");

  let randRgbComponents = [randChar(), randChar(), randChar()];
  body.style.backgroundColor = `rgb(${randRgbComponents[0]}, ${randRgbComponents[1]}, ${randRgbComponents[2]})`;
}

function getRandomColor() {
  let color = `rgb(${randChar()}, ${randChar()}, ${randChar()})`;
  // const letters = "0123456789ABCDEF";
  // for (let i = 0; i < 6; i++) {
  //   color += letters[Math.floor(Math.random() * 16)];
  // }
  return color;
}
