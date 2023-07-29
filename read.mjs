import { readFileSync, writeFileSync, readdirSync } from 'fs';


function inlineHTMLContent(fileName)
{
  let appended = ""
  const allFileContents = readFileSync('./src/pages/html/' + fileName, 'utf-8');
  allFileContents.split(/\r?\n/).forEach(line =>  {
    // console.log(`Line from file: ${line.trim()}`);
    appended += line.trim().replaceAll("\"", `\\"`)
  });

  return appended
}

const files = readdirSync("./src/pages/html")
const pages_h = `
#ifndef PAGES_H
#define PAGES_H

#include <Arduino.h>



${files.map(file => {
  
  return `String ${file.replaceAll(".", "_")} = "${inlineHTMLContent(file)}";\r\n`
}).join('')}

#endif`

console.log(pages_h)

// ${files.map(file => {
//   return `String ${file.replace(".", "_")};\r\n`
// }).join('')}

writeFileSync("./src/pages/pages.h", pages_h)

// let appended = ""
// const allFileContents = readFileSync('./src/html/connect.html', 'utf-8');
// allFileContents.split(/\r?\n/).forEach(line =>  {
//   console.log(`Line from file: ${line.trim()}`);
//   appended += line.trim()
// });

// console.log(appended)

// writeFileSync("./src/html/pages.cpp", appended);



// let pages_cpp = `
// #include "pages.h"

// ${files.map(file => {
  
//   return `String ${file.replaceAll(".", "_")} = "${inlineHTMLContent(file)}";\r\n`
// }).join('')}
// `
// // console.log(pages_cpp)

// writeFileSync("./src/pages/pages.cpp", pages_cpp)
