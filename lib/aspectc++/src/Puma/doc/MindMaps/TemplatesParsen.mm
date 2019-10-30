<map version="0.7.1">
<node TEXT="Templates Parser">
<node TEXT="Caching von Instanzen" POSITION="right">
<node TEXT="Problem: bei Backtracking werden Instanzen oft gleich generiert"/>
<node TEXT="L&#xf6;sung: Caching der Instanz-Unit">
<node TEXT="Markierung als &quot;cached&quot; + POI"/>
<node TEXT="wenn beim Instanziieren schon eine Cache-Instanz mit dem richtigen POI existiert, wird diese verwendet."/>
</node>
</node>
<node TEXT="late instantiation" POSITION="right">
<node TEXT="Instanziierung von Klassen-Templates im Name-Lookup">
<node TEXT="Problem: Zuordnung von Deklaration und Definition bei Nested Classes in Templates"/>
</node>
<node TEXT="Instanziierung von Funktions-Templates bei der Overload Resolution"/>
</node>
<node TEXT="early binding" POSITION="right">
<node TEXT="spezieller Lookup beim Parsen der Instanz"/>
</node>
</node>
</map>
