# Project Description #
Bing for BlackBerry is a SDK that allows intergration and searching Bing in there applications.

This started as a port of [ibing](http://ibing.codeplex.com/) for BlackBerry, when it was discovered that **ibing** is only a partial implementation of the Bing SDK it was expanded to include the full Bing SDK.

Later a major update moved the whole API to Azure, as the old Bing API was being discontinued.

This project has since been **discontinued** due to a major change in communication method with the backend and lack of usage of the library. See "news" at bottom of page.

This project is a full implementation of the Bing SDK, in includes the following:

* Synchronous or Asynchronous querying
* Access to the following SourceTypes:
    * Image
    * News
    * RelatedSearch
    * SpellingSuggestion
    * Video
    * Web

## Bi(n)g News [July 30, 2012] ##

With two days left before the cutover, I finished the new API for Bing searches with Azure. You will need a Azure Account Key which you can get [here](https://datamarket.azure.com/dataset/5BA839F1-12CE-4CCE-BF57-A49D98D29A44) by making a login. The API is still a little rough, but it is purely to get this API out before the *August 1* cutoff. This will be changed over time.

## Shutdown... [December 11, 2016] ##

I had been receiving the emails for a bit (around 2-3 months), but Microsoft announced they were shutting down the Bing Azure Data Market in favor of a new API (akin to the original API before they switched to Azure). This is fine but they're doing one big change: no more XML data. Everything is JSON now. At the time this was originally written, I would've needed to find or port a JSON parser to the library or required an additional library. My goal was a one-stop-shop: reference library, pass in key, make request, get response.

When the library switched from the old APi to Azure, I needed to do some work, but I was still able to have the data structures populate themselves as data was received and parsed. Switching to JSON would make this a lot easier (since it's usually parsed once everything is received), but I've since occupied myself with other projects and goals and... there has been 1 download in the last 30 days. Needless to say, given the low demand for the library, and the effort I would want to put into this (keep the API the same, change the backend, add the translation classes/functions back in) I deemed it would be a sizable amount of work with little payoff. So I've come to the decision to not update the library, which means upon the switch-over to Microsoft's new Bing API, which library will cease to function. It's unfortunate but you need to learn to say "that's enough" and to pick and choose your battles.

If anyone out there relies on this library, please contact me.

Rcmaniac25