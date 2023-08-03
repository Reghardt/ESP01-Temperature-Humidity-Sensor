async function inlineHTMLContent(fileName: string)
{
  let appended = ""
  const content = await Deno.readTextFile(`./src/pages/html/${fileName}`);
  content.split(/\r?\n/).forEach(line =>  {
    appended += line.trim().replaceAll("\"", `\\"`)
  });
  return appended
}

async function parseHTML()
{
  const files = Deno.readDir("./src/pages/html")
  const contents: string[] = [];
  for await (const file of files)
  {
    contents.push(`String ${file.name.replaceAll(".", "_")} = "${await inlineHTMLContent(file.name)}";\r\n`)
  }

  const pages_h = `
  //auto generated file. Run parseHTML.mjs to parse html files
  #ifndef PAGES_H
  #define PAGES_H
  #include <Arduino.h>
  ${contents.map(content => {
    return content
  }).join('')}
  #endif`
    
  Deno.writeTextFile(`./src/pages/pages.h`, pages_h)

  console.log("Deno: HTML Files Parsed")
}

parseHTML();



