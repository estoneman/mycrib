# TODO
* accept context struct instead of explicit arguments to handlers (for ones that
  take arguments)
* let the handler decide which type of operation (GET=SELECT,POST=INSERT,PATCH=UPDATE,DELETE=DELETE)
  * then, create sub handlers for the above operations per endpoint, accordingly
* find way to add routes whose handlers have varying signatures
  * root handler does not need the MHD\_Connection object
