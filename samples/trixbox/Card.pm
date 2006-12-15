package Card;
use strict;

#constructor
sub new {
    my ($class) = @_;
    my $self = {
        _zaptel_file => undef,
        _zapata_file => undef,
	_span_no => undef,
        _card_model => undef,
        _pci_slot  => undef,
        _pci_bus   => undef,
	_fe_cpu	   => 'A',
	_hwec_mode => 'NO',
	_first_chan => '0',
	_zap_context => 'PSTN' 
    };
    bless $self, $class;
    return $self;
}

sub span_no {
    my ( $self, $span_no ) = @_;
    $self->{_span_no} = $span_no if defined($span_no);
    return $self->{_span_no};
}

sub zaptel_file {
   my ( $self, $zaptel_file ) = @_;
   $self->{_zaptel_file} = $zaptel_file if defined($zaptel_file);
   return $self->{_zaptel_file};
}

sub zapata_file {
   my ( $self, $zapata_file ) = @_;
   $self->{_zapata_file} = $zapata_file if defined($zapata_file);
   return $self->{_zapata_file};
}

sub card_model {
    my ( $self, $card_model ) = @_;
    $self->{_card_model} = $card_model if defined($card_model);
    return $self->{_card_model};
}

sub pci_slot {
    my ( $self, $pci_slot ) = @_;
    $self->{_pci_slot} = $pci_slot if defined($pci_slot);
    return $self->{_pci_slot};
}

sub pci_bus {
    my ( $self, $pci_bus ) = @_;
    $self->{_pci_bus} = $pci_bus if defined($pci_bus);
    return $self->{_pci_bus};
}

sub fe_cpu {
    my ( $self, $fe_cpu ) = @_;
    $self->{_fe_cpu } = $fe_cpu if defined($fe_cpu);
    return $self->{_fe_cpu};
}

sub hwec_mode {
    my ( $self, $hwec_mode ) = @_;
    $self->{_hwec_mode} = $hwec_mode if defined($hwec_mode);
    return $self->{_hwec_mode};
}

sub signalling {
    my ( $self, $signalling ) = @_;
    $self->{_signalling} = $signalling if defined($signalling);
    return $self->{_signalling};
}

sub first_chan {
    my ( $self, $first_chan ) = @_;
    $self->{_first_chan} = $first_chan if defined($first_chan);
    return $self->{_first_chan};
}

sub zap_context {
    my ( $self, $zap_context ) = @_;
    $self->{_zap_context} = $zap_context if defined($zap_context);
    return $self->{_zap_context};
}

sub print {
    my ($self) = @_;
    printf (" span_no: %s\n card_model: %s\n pci_slot: %s\n pci_bus:  %s\n hwec_mode: %s\n signalling %s\n first_chan: %s\n", $self->span_no, $self->card_model, $self->pci_slot, $self->pci_bus, $self->hwec_mode, $self->signalling, $self->first_chan);

}


1;
